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

/**
 * Computes n^0.5.
 */
double squared(double n)
{
	double s;

	if (n <= 0)
		return (0);

	s = n;

	while ((s - n/s) > 0.001)
		s = (s + n/s)/2;

	return (s);
}

/**
 * Computes x^y.
 */
double powerd(double x, int y)
{
	double temp;

	if (y == 0)
		return 1;

	temp = powerd(x, y/2);

	if ((y % 2) == 0)
		return (temp*temp);

	if (y > 0)
		return (x*temp*temp);

	return (temp*temp)/x;
}
