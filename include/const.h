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

#ifndef CONST_H_
#define CONST_H_

	#include <stdint.h>

	/**
	 * @name Aliases for Standard C Extensions
	 */
	/**@{*/
	#define inline __inline__  __attribute__((always_inline)) /**< Inline Function */
	#define asm    __asm__                                    /**< Inline Assembly */
	/**@}*/

	/**
	 * @brief Declares something to be unused.
	 *
	 * @param x Thing.
	 */
	#define UNUSED(x) ((void) (x))

	/**
	 * @brief Aligns an object at a boundary.
	 *
	 * @param x Boundary.
	 */
	#define ALIGN(x) __attribute__((aligned(x)))

	/**
	 * @brief Casts something to a uint32_t.
	 *
	 * @param x Something.
	 */
	#define UINT32(x) ((uint32_t)((x) & 0xffffffff))

	/**
	 * @brief Casts something to a float.
	 *
	 * @param x Something.
	 */
	#define FLOAT(x) ((float)(x))

	/**
	 * @name Math Costants
	 */
	/**@{*/
	#define PI 3.14159265359    /**< Pi                 */
	#define E  2.71828182845904 /**< E                  */
	#define SD 0.8              /**< Standard Deviation */
	/**@}*/

#endif /* CONST_H_ */
