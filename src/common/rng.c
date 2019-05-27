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

#include <cap-bench.h>

/**
 * Initializes the pseudo-random number generator.
 */
void rng_initialize(struct rng_state * state)
{
	unsigned n1, n2;

	n1 = (SEED * 104623) % 4294967296;
	n2 = (SEED * 48947)  % 4294967296;

	state->w = (n1) ? n1 : 521288629;
	state->z = (n2) ? n2 : 362436069;
}

/**
 * Returns the next pseudo-random number.
 */
unsigned rng_next(struct rng_state * state)
{
	state->z = 36969 * (state->z & 65535) + (state->z >> 16);
	state->w = 18000 * (state->w & 65535) + (state->w >> 16);

	return ((state->z << 16) + state->w);
}

