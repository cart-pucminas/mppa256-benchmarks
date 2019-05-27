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
#define NTHREADS_MIN                      1  /**< Minimum Number of Working Threads      */
#define NTHREADS_MAX         (NUM_CORES - 1) /**< Maximum Number of Working Threads      */
#define NTHREADS_STEP                     1  /**< Increment on Number of Working Threads */
#define MASKSIZE                          7  /**< Mask Size                              */
#define IMGSIZE       (770 + (MASKSIZE - 1)) /**< Image Size                             */
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
 * @brief Mask.
 */
static double mask[MASKSIZE*MASKSIZE] ALIGN(CACHE_LINE_SIZE);

/**
 * @brief Image.
 */
static unsigned char img[IMGSIZE*IMGSIZE] ALIGN(CACHE_LINE_SIZE);

/**
 * @brief Output image.
 */
static unsigned char output[IMGSIZE*IMGSIZE] ALIGN(CACHE_LINE_SIZE);

/**
 * @brief Indexes the mask.
 */
#define MASK(i, j) mask[(i)*MASKSIZE + (j)]

/**
 * @brief Indexes the image.
 */
#define IMG(i, j) img[(i)*IMGSIZE + (j)]

/**
 * @brief Indexes the output image.
 */
#define OUTPUT(i, j) output[(i)*IMGSIZE + (j)]

/**
 * @brief Dump execution statistics.
 *
 * @param it       Benchmark iteration.
 * @param IMGSIZE  Image size.
 * @param MASKSIZE Mask size.
 * @param stats    Execution statistics.
 */
static inline void benchmark_dump_stats(int it, int imgsize, int masksize, uint64_t *stats)
{
#ifdef NDEBUG
	printf("%s %d %d %d %d %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu\n",
		"[benchmarks][gauss-filter]",
		it,
		NTHREADS,
		imgsize,
		masksize,
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

	printf("%s nthreads=%d imgsize=%d    masksize=%d    time=%.2f s    flops=%.2f Mflops\n",
		"[benchmarks][gauss-filter]",
		NTHREADS,
		imgsize,
		masksize,
		(UINT32(stats[0])/FLOAT(CLUSTER_FREQ)),
		(FLOAT(2*MASKSIZE*MASKSIZE*IMGSIZE*IMGSIZE)/NTHREADS)/(UINT32(stats[0])/FLOAT(CLUSTER_FREQ))
	);
#endif
}

/**
 * @brief Generates the mask.
 */
static inline void generate_mask(void)
{
	int half = MASKSIZE >> 1;;
	double first = 1.0/(2.0*PI*SD*SD);;
	double total = 0.0;

	for (int i = -half; i <= half; i++)
	{
		for (int j = -half; j <= half; j++)
		{
			double sec;

			sec = -((i*i + j*j)/2.0*SD*SD);
			sec = powerd(E, sec);

			MASK(i + half, j + half) = first*sec;
			total += MASK(i + half, j + half);
		}
	}

	for (int i = 0 ; i < MASKSIZE; i++)
	{
		for (int j = 0; j < MASKSIZE; j++)
			MASK(i, j) /= total;
	}
}

/**
 * @brief Applies a gaussian filter to an image.
 *
 * @param i0 Start line.
 * @param in End line.
 */
static inline void gauss_filter(int i0, int in)
{
	int half = MASKSIZE >> 1;

	for (int imgI = i0 + half; imgI < in - half; imgI++)
	{
		for (int imgJ = half; imgJ < IMGSIZE - half; imgJ++)
		{
			double pixel = 0.0;

			for (int maskI = 0; maskI < MASKSIZE; maskI++)
			{
				for (int maskJ = 0; maskJ < MASKSIZE; maskJ++)
				{
					pixel +=
						IMG(imgI + maskI - half, imgJ + maskJ - half)*
						MASK(maskI, maskJ);
				}
			}

			OUTPUT(imgI, imgJ) =
				(pixel > 255) ? 255 : (unsigned char) pixel;
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
			k1b_perf_start(0, k1b_perf_events[j]);

				gauss_filter(i0, in);

			k1b_perf_stop(0);

			stats[j] = k1b_perf_read(0);
		}

		if (i >= SKIP)
			benchmark_dump_stats(i - SKIP, IMGSIZE, MASKSIZE, stats);
	}

	return (NULL);
}

/**
 * @brief Guassian Filter Benchmark Kernel
 *
 * @param nthreads Number of working threads.
 */
static void kernel_gauss_filter(int nthreads)
{
	int nrows;
	pthread_t tid[NTHREADS_MAX];

	/* Save kernel parameters. */
	NTHREADS = nthreads;

	/* Generate mask. */
	generate_mask();

	/* Spawn threads. */
	nrows = IMGSIZE/nthreads;
	for (int i = 0; i < nthreads; i++)
	{
		/* Initialize thread data structure. */
		tdata[i].i0 = nrows*i;
		tdata[i].in = (i == (nthreads - 1)) ? IMGSIZE : (i + 1)*nrows;
		tdata[i].tnum = i;

		pthread_create(&tid[i], NULL, task, &tdata[i]);
	}

	/* Wait for threads. */
	for (int i = 0; i < nthreads; i++)
		pthread_join(tid[i], NULL);
}

/**
 * @brief Gaussian Filter Benchmark
 */
int main(int argc, char **argv)
{
	((void) argc);
	((void) argv);

#ifndef NDEBUG

	kernel_gauss_filter(NTHREADS_MAX);

#else

	for (int nthreads = NTHREADS_MIN; nthreads <= NTHREADS_MAX; nthreads += NTHREADS_STEP)
		kernel_gauss_filter(nthreads);

#endif

	return (0);
}
