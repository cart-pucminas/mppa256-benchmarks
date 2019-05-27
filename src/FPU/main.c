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
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>

#include <cap-bench.h>

/**
 * @name Benchmark Parameters
 */
/**@{*/
#define NTHREADS_MIN               1  /**< Minimum Number of Working Threads      */
#define NTHREADS_MAX  (NUM_CORES - 1) /**< Maximum Number of Working Threads      */
#define NTHREADS_STEP              1  /**< Increment on Number of Working Threads */
#define FLOPS                (100008) /**< Number of Floating Point Operations    */
/**@}*/

/**
 * @brief Task info.
 */
static struct tdata
{
	int tnum;       /**< Thread Number    */
	float scratch;  /**< Scrtch Variable  */
} tdata[NTHREADS_MAX] ALIGN(CACHE_LINE_SIZE);

/**
 * @name Benchmark Kernel Parameters
 */
/**@{*/
static int NTHREADS; /**< Number of Working Threads */
/**@}*/

/**
 * @brief Dump execution statistics.
 *
 * @param it     Benchmark iteration.
 * @param flops  NUmber of float point operations.
 * @param stats  Execution statistics.
 */
static inline void benchmark_dump_stats(int it, int flops, uint64_t *stats)
{
#ifdef NDEBUG
	printf("%s %d %d %d %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu\n",
		"[benchmarks][fpu]",
		it,
		NTHREADS,
		flops,
		UINT32(stats[0]),
		UINT32(stats[1]),
		UINT32(stats[2]),
		UINT32(stats[3]),
		UINT32(stats[4]),
		UINT32(stats[5]),
		UINT32(stats[6]),
		UINT32(stats[7]),
		UINT32(stats[8]),
		UINT32(stats[9]),
		UINT32(stats[10]),
		UINT32(stats[11]),
		UINT32(stats[12]),
		UINT32(stats[13])
	);
#else
	UNUSED(it);

	printf("%s nthreads=%d    time=%.2f s    flops=%.2f MFLOPS\n",
		"[benchmarks][fpu]",
		NTHREADS,
		(UINT32(stats[0])/FLOAT(CLUSTER_FREQ)),
		flops/(UINT32(stats[0])/FLOAT(CLUSTER_FREQ))
	);
#endif
}

/**
 * @brief Perform FPU operations.
 */
static inline int fpu(float scratch)
{
	register float tmp = scratch;

	for (int i = 0; i < FLOPS; i += 9)
	{
		register float k1 = i*1.1;
		register float k2 = i*2.1;
		register float k3 = i*3.1;
		register float k4 = i*4.1;

		tmp += k1 + k2 + k3 + k4;
	}

	return (tmp);
}

/**
 * @brief Perform FPU operations.
 */
static void *task(void *arg)
{
	struct tdata *t = arg;
	uint64_t stats[BENCHMARK_PERF_EVENTS];

	for (int i = 0; i < (NITERATIONS + SKIP); i++)
	{
		for (int j = 0; j < BENCHMARK_PERF_EVENTS; j++)
		{
			k1b_perf_start(0, k1b_perf_events[j]);

				t->scratch = fpu(t->scratch);

			k1b_perf_stop(0);

			stats[j] = k1b_perf_read(0);
		}

		if (i >= SKIP)
			benchmark_dump_stats(i - SKIP, FLOPS, stats);
	}

	return (NULL);
}

/**
 * @brief FPU Benchmark Kernel
 *
 * @param nthreads Number of working threads.
 */
static void kernel_fpu(int nthreads)
{
	pthread_t tid[NTHREADS_MAX];

	/* Save kernel parameters. */
	NTHREADS = nthreads;

	/* Spawn threads. */
	for (int i = 0; i < nthreads; i++)
	{
		/* Initialize thread data structure. */
		tdata[i].scratch = 0.0;
		tdata[i].tnum = i;

		pthread_create(&tid[i], NULL, task, &tdata[i]);
	}

	/* Wait for threads. */
	for (int i = 0; i < nthreads; i++)
		pthread_join(tid[i], NULL);
}

/**
 * @brief FPU Benchmark
 */
int main(int argc, char **argv)
{
	((void) argc);
	((void) argv);


#ifndef NDEBUG

	kernel_fpu(NTHREADS_MAX);

#else

	for (int nthreads = NTHREADS_MIN; nthreads <= NTHREADS_MAX; nthreads += NTHREADS_STEP)
		kernel_fpu(nthreads);

#endif

	return (0);
}
