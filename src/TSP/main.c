/*
 * Copyright (C) 2013-2019 The Engineers of CAP Bench
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <mppa/osconfig.h>
#include <assert.h>
#include <limits.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <string.h>
#include <unistd.h>

#include <cap-bench.h>

/*============================================================================*
 * General configuration                                                      *
 *============================================================================*/

/**
 * @name Benchmark Parameters
 */
/**@{*/
#define NTHREADS_MIN               1  /**< Minimum Number of Working Threads      */
#define NTHREADS_MAX  (NUM_CORES - 2) /**< Maximum Number of Working Threads      */
#define NTHREADS_STEP              1  /**< Increment on Number of Working Threads */
#define NTOWNS                    10  /**< Number of Towns.                       */
#define NPARTITIONS               20  /**< Partitions Per Cluster                 */
/**@}*/

/**
 * @name Other Parameters
 */
/**@{*/
#define MAX_GRID_X		                                   100   /**< Maximum of Lines on Grid            */
#define MAX_GRID_Y		                                   100   /**< Maximum of Columns on Grid          */
#define INITIAL_JOB_DIST                                    50.0 /**< Initial Job Distribution Percentage */
#define MIN_JOBS_PER_THREAD                                 20   /**< Minimum Jobs Per Thread             */
#define MAX_JOBS_PER_THREAD                                150   /**< Maximum Jobs per Thread             */
#define MAX_JOBS_PER_QUEUE  (MAX_JOBS_PER_THREAD * NTHREADS_MAX) /**< Maximum of Jobs Per Thread          */
/**@}*/

/**
 * @brief Current performance event being monitored.
 */
static int perf;

/**
 * @name Current Benchmark Parameters
 */
/**@{*/
static int NTHREADS;    /**< Number of Working Threads. */
/**@}*/

/**
 * @name Benchmark Kernel Variables
 */
/**@{*/
static int max_hops;              /**< Maximum number of hops.              */
static int next_partition_id;     /**< Next partition interval ID.          */
static int processed_partitions;  /**< Amount of intervals processed.       */
static int min_distance;          /**< Minimum distance found.              */
/**@}*/

/**
 * @name Concurrency control
 */
/**@{*/
static pthread_mutex_t main_lock; /**< Lock for Kernel Variables.           */
static int waiting_threads = 0;   /**< Number of threads waiting the queue. */
/**@}*/

static uint64_t stats[NTHREADS_MAX][BENCHMARK_PERF_EVENTS] ALIGN(CACHE_LINE_SIZE);

/*----------------------------------------------------------------------------*
 * benchmark_dump_stats()                                                     *
 *----------------------------------------------------------------------------*/

/**
 * @brief Dump execution statistics.
 *
 * @param it      Benchmark iteration.
 * @param ntowsn  Number of towns.
 * @param stats   Execution statistics.
 */
static void benchmark_dump_stats(int it, int ntowns, uint64_t *st)
{
#ifdef NDEBUG
	printf("%s %d %d %d %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu\n",
		"[benchmarks][tsp]",
		it,
		NTHREADS,
		ntowns,
		UINT32(st[0]),
		UINT32(st[1]),
		UINT32(st[2]),
		UINT32(st[3]),
		UINT32(st[4]),
		UINT32(st[5]),
		UINT32(st[6]),
		UINT32(st[7]),
		UINT32(st[8]),
		UINT32(st[9]),
		UINT32(st[10]),
		UINT32(st[11]),
		UINT32(st[12]),
		UINT32(st[13])
	);
#else
	UNUSED(it);

	printf("%s nthreads=%d    ntowns=%d    min_distance=%d    time=%.2f us\n",
		"[benchmarks][tsp]",
		NTHREADS,
		ntowns,
		min_distance,
		(UINT32(st[0])/FLOAT(CLUSTER_FREQ))
	);
#endif
}

/*============================================================================*
 * Job Queue                                                                  *
 *============================================================================*/

struct partition_interval
{
	int start;
	int end;
};

static struct distance_matrix
{
	int to_city;
	int dist;
} distance[NTOWNS][NTOWNS];

static struct jobs_queue
{
	int begin;
	int end;
	int max_size;

	sem_t semaphore;
	pthread_mutex_t lock;

	enum queue_status
	{
		CLOSED_QUEUE = 0,
		WAIT_QUEUE   = 1,
		EMPTY_QUEUE  = 2
	} status;

	struct job
	{
		int lenght;
		int path[NTOWNS];
	} jobs[MAX_JOBS_PER_QUEUE];
} queue;

/*----------------------------------------------------------------------------*
 * reset_queue()                                                              *
 *----------------------------------------------------------------------------*/

static void reset_queue(void)
{
	queue.begin = 0;
	queue.end   = 0;
}

/*----------------------------------------------------------------------------*
 * init_queue()                                                               *
 *----------------------------------------------------------------------------*/

void init_queue(int queue_size)
{
	queue.begin    = 0;
	queue.end      = 0;
	queue.status   = EMPTY_QUEUE;
	queue.max_size = queue_size;

	sem_init(&queue.semaphore, 0, 0);

	pthread_mutex_init(&queue.lock, NULL);

	memset(&queue.jobs[0], 0, sizeof(struct job) * queue_size);

	waiting_threads = 0;
}

/*----------------------------------------------------------------------------*
 * close_queue()                                                              *
 *----------------------------------------------------------------------------*/

static void close_queue(void)
{
	queue.status = CLOSED_QUEUE;

	pthread_mutex_lock(&main_lock);

		for (int i = 0; i < waiting_threads; i++)
			sem_post(&queue.semaphore);

	pthread_mutex_unlock(&main_lock);
}

/*----------------------------------------------------------------------------*
 * enqueue()                                                                  *
 *----------------------------------------------------------------------------*/

static void enqueue(struct job *job)
{
	pthread_mutex_lock(&queue.lock);

		assert(queue.end < queue.max_size);

		memcpy(&queue.jobs[queue.end], job, sizeof(struct job));

		queue.end++;

		sem_post(&queue.semaphore);

	pthread_mutex_unlock(&queue.lock);
}

/*----------------------------------------------------------------------------*
 * dequeue()                                                                  *
 *----------------------------------------------------------------------------*/

static int repopulate_queue(void);

static int dequeue(struct job * job)
{
	int index;
	int jobs_added;

	pthread_mutex_lock(&queue.lock);

		dcache_invalidate();

		while (queue.begin == queue.end)
		{
			switch (queue.status)
			{
				case CLOSED_QUEUE:
					pthread_mutex_unlock(&queue.lock);
					return (0);

				case WAIT_QUEUE:
					pthread_mutex_unlock(&queue.lock);

						pthread_mutex_lock(&main_lock);
						dcache_invalidate();

							waiting_threads++;

						dcache_invalidate();
						pthread_mutex_unlock(&main_lock);

						sem_wait(&queue.semaphore);

						pthread_mutex_lock(&main_lock);
						dcache_invalidate();

							waiting_threads++;

						dcache_invalidate();
						pthread_mutex_unlock(&main_lock);

					pthread_mutex_lock(&queue.lock);
					break;

				case EMPTY_QUEUE:
					queue.status = WAIT_QUEUE;
					reset_queue();

					pthread_mutex_unlock(&queue.lock);

						jobs_added = repopulate_queue();

					pthread_mutex_lock(&queue.lock);

					if (jobs_added)
						queue.status = EMPTY_QUEUE;
					else
						close_queue();

					break;

				/* Unreachable. */
				default:
					assert(0);
					break;
			}
		}

		index = queue.begin++;

		memcpy(job, &queue.jobs[index], sizeof(struct job));

		dcache_invalidate();

	pthread_mutex_unlock(&queue.lock);

	return (1);
}

/*============================================================================*
 * TSP functions                                                              *
 *============================================================================*/

static inline int present(int city, int hops, int *path)
{
	for (int i = 0; i < hops; i++)
	{
		if (path[i] == city)
			return (1);
	}

	return (0);
}

/*----------------------------------------------------------------------------*
 * get_next_partition()                                                       *
 *----------------------------------------------------------------------------*/

static void get_next_partition(struct partition_interval * partition)
{
	int alfa;
	int block_size2;
	int block_size;

	alfa = 1;
	block_size = NPARTITIONS / (1.0 / (INITIAL_JOB_DIST / 100.0));

	if (processed_partitions != 0)
	{
		block_size2 = (NPARTITIONS - next_partition_id) / alfa;
		block_size = (block_size2 < block_size) ? block_size2 : block_size;
	}

	if (block_size < 1)
		block_size = 1;

	partition->start = partition->end = -1;

	if (next_partition_id < NPARTITIONS)
		partition->start = next_partition_id;

	if (next_partition_id + block_size - 1 < NPARTITIONS)
		partition->end = next_partition_id + block_size - 1;
	else
		partition->end = partition->start + NPARTITIONS - partition->start;

	next_partition_id += block_size;
}

/*----------------------------------------------------------------------------*
 * distributor()                                                              *
 *----------------------------------------------------------------------------*/

static void distributor(
	struct partition_interval *partition,
	int hops,
	int lenght,
	int *path,
	int *jobs_count
)
{
	int me;
	int city;
	int dist;
	int job_id;
	struct job new_job;

	/* End recursion */
	if (hops == max_hops)
	{
		job_id = (*jobs_count) % NPARTITIONS;

		if ((job_id >= partition->start) && (job_id <= partition->end))
		{
			new_job.lenght = lenght;

			for (int i = 0; i < hops; i++)
				new_job.path[i] = path[i];

			enqueue(&new_job);
		}

		(*jobs_count)++;
	}

	/* Go down */
	else
	{
		me = path[hops - 1];

		for (int i = 0; i < NTOWNS; i++)
		{
			city = distance[me][i].to_city;

			if (!present(city, hops, path))
			{
				path[hops] = city;
				dist = distance[me][i].dist;

				distributor(partition, (hops + 1), (lenght + dist), path, jobs_count);
			}
		}
	}
}

/*----------------------------------------------------------------------------*
 * repopulate_queue()                                                         *
 *----------------------------------------------------------------------------*/

static int repopulate_queue(void)
{
	int jobs_count;
	int path[NTOWNS];
	struct partition_interval partition;

	get_next_partition(&partition);

	if (partition.start < 0)
		return (0);

	processed_partitions += (partition.end - partition.start + 1);

	/* Generate jobs. */

	path[0] = 0;
	jobs_count = 0;

	distributor(&partition, 1, 0, path, &jobs_count);

	return (1);
}

/*----------------------------------------------------------------------------*
 * get_shortest_lenght()                                                      *
 *----------------------------------------------------------------------------*/

static int get_shortest_lenght(void)
{
	int dist;

	pthread_mutex_lock(&main_lock);
	dcache_invalidate();

		dist = min_distance;

	dcache_invalidate();
	pthread_mutex_unlock(&main_lock);

	return (dist);
}

/*----------------------------------------------------------------------------*
 * update_minimum_distance()                                                  *
 *----------------------------------------------------------------------------*/

static int update_minimum_distance(int new_distance)
{
	int updated;
	int old_distance;

	updated = 0;

	pthread_mutex_lock(&main_lock);
	dcache_invalidate();

		old_distance = min_distance;

		if (new_distance < old_distance)
		{
			min_distance = new_distance;

			updated = 1;
			dcache_invalidate();
		}

	pthread_mutex_unlock(&main_lock);

	return (updated);
}

/*----------------------------------------------------------------------------*
 * execute_tsp()                                                              *
 *----------------------------------------------------------------------------*/

static void execute_tsp(int hops, int lenght, int *path)
{
	int me;
	int city;
	int dist;

	if (lenght >= get_shortest_lenght())
		return;

	/* End recursion. */
	if (hops == NTOWNS)
		update_minimum_distance(lenght);

	/* Go down. */
	else
	{
		me = path[hops - 1];

		for (int i = 0; i < NTOWNS; i++)
		{
			city = distance[me][i].to_city;

			if (!present(city, hops, path))
			{
				path[hops] = city;
				dist = distance[me][i].dist;
				execute_tsp((hops + 1), (lenght + dist), path);
			}
		}
	}
}

/*============================================================================*
 * Worker                                                                     *
 *============================================================================*/

/**
 * @brief Gets TSP jobs.
 */
static void *worker(void *arg)
{
	int found;
	struct job job;
	int tid = ((int) arg);

	dcache_invalidate();

	k1b_perf_start(0, k1b_perf_events[perf]);

		while (1)
		{
			found = dequeue(&job);

			if (!found)
				break;

			execute_tsp(max_hops, job.lenght, job.path);
		}

	k1b_perf_stop(0);
	stats[tid][perf] = k1b_perf_read(0);

	dcache_invalidate();

	return (NULL);
}

/*============================================================================*
 * Main thread                                                                *
 *============================================================================*/

static void init_distance(void)
{
	int tmp;
	int city;
	int dx, dy;
	int x[NTOWNS], y[NTOWNS], tempdist[NTOWNS];
	struct rng_state rand_state;

	city = 0;

	rng_initialize(&rand_state);

	for (int i = 0; i < NTOWNS; i++)
	{
		x[i] = rng_next(&rand_state) % MAX_GRID_X;
		y[i] = rng_next(&rand_state) % MAX_GRID_Y;
	}

	for (int i = 0; i < NTOWNS; i++)
	{
		for (int j = 0; j < NTOWNS; j++)
		{
			dx = x[i] - x[j];
			dy = y[i] - y[j];
			tempdist[j] = ((int) squared((double) ((dx * dx) + (dy * dy))));
		}

		for (int j = 0; j < NTOWNS; j++)
		{
			tmp = INT_MAX;

			for (int k = 0; k < NTOWNS; k++)
			{
				if (tempdist[k] < tmp)
				{
					tmp = tempdist[k];
					city = k;
				}
			}

			tempdist[city] = INT_MAX;
			distance[i][j].to_city = city;
			distance[i][j].dist    = tmp;
		}
	}
}

/*----------------------------------------------------------------------------*
 * init_max_hops()                                                            *
 *----------------------------------------------------------------------------*/

static int init_max_hops(void)
{
	int total;
	int new_total;

	total = 1;
	new_total = 1;
	max_hops = 0;

	while ((new_total < (MIN_JOBS_PER_THREAD * NTHREADS)) && (max_hops < (NTOWNS - 1)))
	{
		max_hops++;
		total = new_total;
		new_total *= (NTOWNS - max_hops);
	}

	return (total);
}

/*----------------------------------------------------------------------------*
 * init_tsp()                                                                 *
 *----------------------------------------------------------------------------*/

static void init_tsp(int nthreads, int ntowns)
{
	int qsize;

	((void) ntowns);

	pthread_mutex_init(&main_lock, NULL);

	NTHREADS     = nthreads;
	min_distance = INT_MAX;
	next_partition_id = 0;
	processed_partitions = 0;

	init_distance();

	qsize = init_max_hops();
	qsize = qsize + qsize;

	init_queue(qsize);

	dcache_invalidate();
}

/*----------------------------------------------------------------------------*
 * finish_tsp()                                                               *
 *----------------------------------------------------------------------------*/

static void finish_tsp(void)
{
	dcache_invalidate();

	sem_destroy(&queue.semaphore);
	pthread_mutex_destroy(&queue.lock);
	pthread_mutex_destroy(&main_lock);
}

/*============================================================================*
 * kernel_tsp()                                                               *
 *============================================================================*/

/**
 * @brief Travelling Salesman Benchmark Kernel
 *
 * @param nthreads Number of working threads.
 * @param ntowns   Number of towns.
 */
static void kernel_tsp(int nthreads, int ntowns)
{
	for (int k = 0; k < (NITERATIONS + SKIP); k++)
	{
		for (perf = 0; perf < BENCHMARK_PERF_EVENTS; perf++)
		{
			pthread_t tid[NTHREADS_MAX];

			/* Save kernel parameters. */
			init_tsp(nthreads, ntowns);

			/* Spawn threads. */
			for (int i = 0; i < nthreads; i++)
				pthread_create(&tid[i], NULL, worker, (void *) i);

			/* Wait for threads. */
			for (int i = 0; i < nthreads; i++)
				pthread_join(tid[i], NULL);

			/* House keeping. */
			finish_tsp();
		}

		/* Dump statistics. */
		if (k >= SKIP)
		{
			for (int i = 0; i < nthreads; i++)
				benchmark_dump_stats(k - SKIP, ntowns, &stats[i][0]);
		}
	}
}

/**
 * @brief TSP Benchmark
 */
int main(int argc, char **argv)
{
	((void) argc);
	((void) argv);

#ifndef NDEBUG

	kernel_tsp(NTHREADS_MAX, NTOWNS);

#else

	for (int nthreads = NTHREADS_MIN; nthreads <= NTHREADS_MAX; nthreads += NTHREADS_STEP)
		kernel_tsp(nthreads, NTOWNS);

#endif

	return (0);
}
