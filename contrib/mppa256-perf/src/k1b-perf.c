/*
 * MIT License
 *
 * Copyright(c) 2019 Pedro Henrique Penna <pedrohenriquepenna@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <k1b-perf.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Asserts a valid performance monitor.
 *
 * @param perf Monitor to assert.
 *
 * @returns True if the performance monitor is valid and false
 * otherwise.
 */
static __inline__ bool k1b_perf_monitor_is_valid(int perf)
{
	return ((perf >= 0) && (perf < K1B_PERF_MONITORS_NUM));
}

/**
 * @brief Asserts a valid performance event.
 *
 * @param event Event to assert.
 *
 * @returns True if the eventormance event is valid and false
 * otherwise.
 */
static __inline__ bool k1b_perf_event_is_valid(int event)
{
	return ((event >= 0) && (event < K1B_PERF_EVENTS_NUM));
}

/**
 * @brief Writes to PMC register.
 *
 * @param pmc Value for PMC register.
 */
static __inline__ void k1b_pmc_write(uint32_t pmc)
{
	__asm__ __volatile__ ("set $pmc, %0;;" :: "r" (pmc));
}

/**
 * @brief Reads the PMC register.
 *
 * @returns The value of the PMC register
 */
static __inline__ uint32_t k1b_pmc_read(void)
{
	uint32_t pmc;

	__asm__ __volatile__ ("get %0, $pmc;;" : "=r" (pmc));

	return (pmc);
}

/**
 * @brief Resets a performance monitor.
 *
 * @param perf Target performance counter.
 *
 * @returns The previous value of the performance counter @p PMC
 * register.
 */
static uint32_t k1b_perf_reset(int perf)
{
	uint32_t pmc;
	uint32_t pmX, pmY;

	/* Chain mode. */
	pmY = (pmX = perf << 1) + 1;

	/* Reset performance monitor. */
	pmc = k1b_pmc_read();
	pmc &= ~(K1B_PERF_MASK(pmX) | K1B_PERF_MASK(pmY));
	pmc |= K1B_PERF_RESET(pmX) | K1B_PERF_RESET(pmY);
	k1b_pmc_write(pmc);

	return (pmc);
}

/**
 * The k1b_perf_start() function starts watching for the @p event
 * using the performance monitor @p perf.
 */
int k1b_perf_start(int perf, int event)
{
	uint32_t pmc;
	uint32_t pmX, pmY;

	/* Invalid performance monitor. */
	if (!k1b_perf_monitor_is_valid(perf))
		return (-EINVAL);

	/* Invalid event. */
	if (!k1b_perf_event_is_valid(event))
		return (-EINVAL);

	/* Chain mode. */
	pmY = (pmX = perf << 1) + 1;

	/* Reset performance monitor and then start it . */
	pmc = k1b_perf_reset(perf);
	pmc &= ~(K1B_PERF_MASK(pmX) | K1B_PERF_MASK(pmY));
	pmc |= K1B_PERF_START(pmX, event) | K1B_PERF_START(pmY, event);
	k1b_pmc_write(pmc);

	return (0);
}

/**
 * The k1b_perf_stop() function stops the performance monitor @p perf.
 */
int k1b_perf_stop(int perf)
{
	uint32_t pmc;
	uint32_t pmX, pmY;

	/* Invalid performance monitor. */
	if (!k1b_perf_monitor_is_valid(perf))
		return (-EINVAL);

	/* Chain mode. */
	pmY = (pmX = perf << 1) + 1;

	/* Stop performance monitor. */
	pmc = k1b_pmc_read();
	pmc &= ~(K1B_PERF_MASK(pmX) | K1B_PERF_MASK(pmY));
	pmc |= K1B_PERF_STOP(pmX) | K1B_PERF_STOP(pmY);
	k1b_pmc_write(pmc);

	return (0);
}

/**
 * The k1b_perf_setup() function initializes performance monitors. It
 * stops al monitors and setup them to be disabled in idle, privileged
 * and debug modes.
 */
void k1b_perf_setup(void)
{
	uint32_t pmc;

	/* Stop all monitors. */
	pmc = K1B_PERF_STOP(0) | K1B_PERF_STOP(1) |
		K1B_PERF_STOP(2) | K1B_PERF_STOP(3);

	/*
	 * - Disengage Monitors Timer
	 * - Stop Monitors in Privileged Mode
	 * - Stop Monitors in Debug
	 * - Enable PM0 and PM1 Chaining
	 * - Enable PM2 and PM3 Chaining
	 */
	pmc |= K1B_PERF_SMD | K1B_PERF_SMP | K1B_PERF_DMC |
		  K1B_PERF_CHAIN_0_1 | K1B_PERF_CHAIN_2_3;

	/* Write configuration. */
	k1b_pmc_write(pmc);

	/* Reset performance monitors. */
	pmc = k1b_perf_reset(K1B_PERF_PM_0_1);
	pmc = k1b_perf_reset(K1B_PERF_PM_2_3);
}
