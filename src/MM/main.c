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
#define MATSIZE                   84  /**< Matrix Size                            */
/**@}*/

/**
 * @name Benchmark Kernel Parameters
 */
/**@{*/
static int NTHREADS; /**< Number of Working Threads */
/**@}*/

/**
 * @brief Task info.
 */
struct tdata
{
	int tnum; /**< Thread Number */
	int i0;   /**< Start Line    */
	int in;   /**< End Line      */
} tdata[NTHREADS_MAX] ALIGN(CACHE_LINE_SIZE);

/**
 * @brief Matrices.
 */
/**@{*/
static float a[MATSIZE*MATSIZE] ALIGN(CACHE_LINE_SIZE);
static float b[MATSIZE*MATSIZE] ALIGN(CACHE_LINE_SIZE);
static float ret[MATSIZE*MATSIZE] ALIGN(CACHE_LINE_SIZE);
/**@}*/

/**
 * @brief Dump execution statistics.
 *
 * @param it      Benchmark iteration.
 * @param matsize Matrix size.
 * @param stats   Execution statistics.
 */
static inline void benchmark_dump_stats(int it, int matsize, uint64_t *stats)
{
#ifdef NDEBUG
	printf("%s %d %d %d %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu\n",
		"[benchmarks][matrix]",
		it,
		NTHREADS,
		matsize,
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

	printf("%s nthreads=%d matsize=%d    time=%.2f s    flops=%.2f MFLOPS\n",
		"[benchmarks][matrix]",
		NTHREADS,
		matsize,
		(UINT32(stats[0])/FLOAT(CLUSTER_FREQ)),
		(FLOAT(2*matsize*matsize*matsize)/NTHREADS)/(UINT32(stats[0])/FLOAT(CLUSTER_FREQ))
	);
#endif
}

/**
 * @brief Initializes a chunk of the matrix.
 *
 * @param i0 Start line.
 * @param in End line.
 */
static inline void matrix_init(int i0, int in)
{
	for (int i = i0; i < in; i++)
	{
		for (int j = 0; j < MATSIZE; j++)
		{
			a[i*MATSIZE + j] = 1.0;
			b[i*MATSIZE + j] = 1.0;
		}
	}
}

/**
 * @brief Multiples a chunk of the matrices.
 *
 * @param i0 Start line.
 * @param in End line.
 */
static inline void matrix_mult(int i0, int in)
{
	for (int i = i0; i < in; ++i)
	{
		int ii = i*MATSIZE;

		for (int j = 0; j < MATSIZE; ++j)
		{
			float c = 0;

			for (int k = 0; k < MATSIZE; k += 4)
			{
				c += a[ii + k]*b[k*MATSIZE + j];
				c += a[ii + k + 1]*b[k*MATSIZE + j + 1];
				c += a[ii + k + 2]*b[k*MATSIZE + j + 2];
				c += a[ii + k + 3]*b[k*MATSIZE + j + 3];
			}

			ret[ii + j] = c;
		}
	}
}

/**
 * @brief Multiplies matrices.
 */
static void *task(void *arg)
{
	struct tdata *t = arg;
	int i0 = t->i0;
	int in = t->in;
	uint64_t stats[BENCHMARK_PERF_EVENTS];

	stats[0] = UINT64_MAX;

	for (int i = 0; i < (NITERATIONS + SKIP); i++)
	{
		for (int j = 0; j < BENCHMARK_PERF_EVENTS; j++)
		{
			matrix_init(i0, in);

			k1b_perf_start(0, k1b_perf_events[j]);

				matrix_mult(i0, in);

			k1b_perf_stop(0);

			stats[j] = k1b_perf_read(0);
		}

		if (i >= SKIP)
			benchmark_dump_stats(i - SKIP, MATSIZE, stats);
	}

	return (NULL);
}

/**
 * @brief Matrix Multiplication Benchmark Kernel
 *
 * @param nthreads Number of working threads.
 */
static void kernel_matrix(int nthreads)
{
	int nrows;
	pthread_t tid[NTHREADS_MAX];

	/* Save kernel parameters. */
	NTHREADS = nthreads;

	nrows = MATSIZE/nthreads;

	/* Spawn threads. */
	for (int i = 0; i < nthreads; i++)
	{
		/* Initialize thread data structure. */
		tdata[i].i0 = nrows*i;
		tdata[i].in = (i == (nthreads - 1)) ? MATSIZE : (i + 1)*nrows;
		tdata[i].tnum = i;

		pthread_create(&tid[i], NULL, task, &tdata[i]);
	}

	/* Wait for threads. */
	for (int i = 0; i < nthreads; i++)
		pthread_join(tid[i], NULL);
}

/**
 * @brief Matrix Multiplication Benchmark
 */
int main(int argc, char **argv)
{
	((void) argc);
	((void) argv);

#ifndef NDEBUG

	kernel_matrix(NTHREADS_MAX);

#else

	for (int nthreads = NTHREADS_MIN; nthreads <= NTHREADS_MAX; nthreads += NTHREADS_STEP)
		kernel_matrix(nthreads);

#endif

	return (0);
}
