/***************************************************************************
 *   Energex                                                               *
 *                                                                         *
 *   Copyright (C) 2008-2010 by Markus Walser                              *
 *   markus.walser@gmail.com                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

/*! 
 * @file        ntc.c
 * @brief       NTC temperature sensor linearisation.
 * @author      Markus Walser (markus.walser@gmail.com)
 * @date        29.11.2009
 */

#include "ntc.h"
#include <avr/io.h>
#include <avr/pgmspace.h>

#define COUNT_OF(A) (sizeof(A)/sizeof(A[0]))

prog_int16_t ntc_table[] =
{         // Vref = 3.065V 
   1996,  // 1996.433   -40°C    0
   1978,  // 1977.544   -35°C    1
   1952,  // 1952.459   -30°C    2
   1920,  // 1919.871   -25°C    3
   1878,  // 1878.190   -20°C    4
   1826,  // 1825.799   -15°C    5
   1762,  // 1761.626   -10°C    6
   1684,  // 1684.304    -5°C    7
   1594,  // 1594.470     0°C    8
   1494,  // 1494.117     5°C    9
   1384,  // 1383.774    10°C   10
   1266,  // 1265.552    15°C   11
   1144,  // 1143.650    20°C   12
   1022,  // 1021.667    25°C   13
    903,  //  903.176    30°C   14
    791,  //  790.907    35°C   15
    687,  //  686.980    40°C   16
    593,  //  592.978    45°C   17 
    509,  //  509.242    50°C   18 
    435,  //  434.587    55°C   19 
    370,  //  369.554    60°C   20 
    315,  //  315.005    65°C   21 
    268,  //  268.436    70°C   22 
    229,  //  228.618    75°C   23 
    195,  //  194.865    80°C   24 
    166,  //  166.167    85°C   25 
    142,  //  141.904    90°C   26 
    121,  //  121.480    95°C   27 
    104,  //  104.209   100°C   28 
     90,  //   89.580   105°C   29 
     77,  //   77.172   110°C   30 
     67,  //   66.730   115°C   31 
     58,  //   57.837   120°C   32 
     50   //   50.226   125°C   33 
};

static int8_t ntc_log_search(const uint16 adc);
static int16_t ntc_read_table(uint8_t idx);

int16_t ntc_get_temp(const uint16 adc)
{
	if (adc > ntc_read_table(0))
	{
		return -4000;
	}
	else if (adc < ntc_read_table(COUNT_OF(ntc_table)-1))
	{
		return 12500;
	}

	int8_t idx = ntc_log_search(adc);

	int16_t lower = ntc_read_table(idx);
	int16_t diff = lower - ntc_read_table(idx+1);

	uint16_t temp = 500 * (lower - adc);
	temp /= diff ;
	temp += 500*idx;

	return (int16_t)temp - 4000;
}

int16_t ntc_read_table(uint8_t idx)
{
	return pgm_read_word(ntc_table + idx);
}

int8_t ntc_log_search(const uint16 adc)
{
	int8_t mid;
	int8_t low  = 0;
	int8_t high = COUNT_OF(ntc_table)-1;
	
	for (;;)
	{
		mid  = (low + high) >> 1;

		if (ntc_read_table(mid) < adc)
		{
			high = mid - 1;
		}
		else if (ntc_read_table(mid+1) > adc)
		{
			low = mid + 1;
		}
		else
		{
			return mid;
		}
	}
}

