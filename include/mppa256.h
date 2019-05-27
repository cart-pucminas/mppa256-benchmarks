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

#ifndef MPPA256_H_
#define MPPA256_H_

	#include <k1b-perf.h>

	/**
	 * @brief Cluster frequency (in MHz)
	 */
	#define CLUSTER_FREQ 400

	/**
	 * @brief Number of cores
	 */
	#ifdef __node__
		#define NUM_CORES 16
	#else
		#define NUM_CORES 4
	#endif

	/**
	 * @brief Cache line size log2.
	 */
	#define CACHE_LINE_SIZE_LOG2 6

	/**
	 * @brief Cache size log2.
	 */
	#define CACHE_SIZE_LOG2 13

	/**
	 * @brief Cache line size (in bytes).
	 */
	#define CACHE_LINE_SIZE (1 << CACHE_LINE_SIZE_LOG2)

	/**
	 * @brief Cache size (in bytes).
	 */
	#define CACHE_SIZE (1 << CACHE_SIZE_LOG2)

	/**
	 * @brief Invalidates the data cache of the underlying core.
	 */
	static inline void dcache_invalidate(void)
	{
		__builtin_k1_wpurge();
		__builtin_k1_fence();
		__builtin_k1_dinval();
	}

#endif /* MPPA256_H_ */

