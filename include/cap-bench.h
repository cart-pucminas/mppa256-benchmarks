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

#ifndef CAP_BENCH_H_
#define CAP_BENCH_H_

	#include <mppa256.h>
	#include <config.h>
	#include <const.h>

	/**
	 * @brief Number of events to profile.
	 */
	#define BENCHMARK_PERF_EVENTS K1B_PERF_EVENTS_NUM

	/**
	 * @brief Seed for pseudo-random number generator.
	 */
	#define SEED 12345

	/**
	 * @brief Random number generator state.
	 */
	struct rng_state
	{
		unsigned w;
		unsigned z;
	};

	/**
	 * @brief Computes the square root of a number.
	 *
	 * @param n Number.
	 *
	 * @returns The square root of @param n.
	 */
	extern double squared(double n);

	/**
	 * @brief Raises a number to a power.
	 *
	 * @param x Number.
	 * @param y Power.
	 *
	 * @returns x^y.
	 */
	extern double powerd(double x, int y);

	/**
	 * @brief Initializes the pseudo-random number generator.
	 *
	 * @param state Store location for the state of the random-number generator.
	 */
	extern void rng_initialize(struct rng_state *state);

	/**
	 * @brief Returns a pseudo-random number.
	 *
	 * @param state State of the random-number generator.
	 *
	 * @param A pseudo-randum number.
	 */
	extern unsigned rng_next(struct rng_state *state);

	/**
	 * Performance events.
	 */
	extern int k1b_perf_events[BENCHMARK_PERF_EVENTS];

#endif /* CAP_BENCH_H_ */
