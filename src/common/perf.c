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
 * Performance events.
 */
int k1b_perf_events[BENCHMARK_PERF_EVENTS] = {
	K1B_PERF_CYCLES,
	K1B_PERF_ICACHE_HITS,
	K1B_PERF_ICACHE_MISSES,
	K1B_PERF_ICACHE_STALLS,
	K1B_PERF_DCACHE_HITS,
	K1B_PERF_DCACHE_MISSES,
	K1B_PERF_DCACHE_STALLS,
	K1B_PERF_BUNDLES,
	K1B_PERF_BRANCH_TAKEN,
	K1B_PERF_BRANCH_STALLS,
	K1B_PERF_REG_STALLS,
	K1B_PERF_ITLB_STALLS,
	K1B_PERF_DTLB_STALLS,
	K1B_PERF_STREAM_STALLS
};
