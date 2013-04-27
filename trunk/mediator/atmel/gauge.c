/***************************************************************************
 *   Energex                                                               *
 *                                                                         *
 *   Copyright (C) 2008-2011 by Markus Walser                              *
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
 * @file        gauge.c
 * @brief       Lithium polymer gauge.
 * @author      Markus Walser (markus.walser@gmail.com)
 * @date        20.12.2010
 */

#include <avr/pgmspace.h>
#include "global.h"
#include "gauge.h"

#define COUNT_OF(A) (sizeof(A)/sizeof(A[0]))

// off-load voltage
const uint16_t gauge_table[] PROGMEM =
{
   4170,  // 25.0Ah  0  100%
   4000,  // 22.5Ah  1   90%
   3880,  // 20.0Ah  2   80%
   3790,  // 17.5Ah  3   70%
   3700,  // 15.0Ah  4   60%
   3660,  // 12.5Ah  5   50%
   3600,  // 10.0Ah  6   40%
   3570,  //  7.5Ah  7   30%
   3520,  //  5.0Ah  8   20%
   3460,  //  2.5Ah  9   10%
   3200,  //  0.0Ah  10   0%
};

static int16_t gauge_read_table(uint8_t idx);
static int8_t  gauge_log_search(const uint16 voltage);

int16_t gauge_get_capacity(uint16_t voltage)
{
        if (voltage > gauge_read_table(0))
        {
                return 2500;
        }
        else if (voltage < gauge_read_table(COUNT_OF(gauge_table)-1))
        {
                return 0;
        }

        int8_t idx = gauge_log_search(voltage);

        int16_t lower = gauge_read_table(idx);
        int16_t diff = lower - gauge_read_table(idx+1);

        uint16_t temp = 250 * (lower - voltage);
        temp /= diff ;
	temp = 2500 - temp;
        temp -= 250*idx;

        return (int16_t)temp;
}

int16_t gauge_read_table(uint8_t idx)
{
	return pgm_read_word(gauge_table + idx);
}

int8_t gauge_log_search(const uint16 voltage)
{
	int8_t mid;
	int8_t low  = 0;
	int8_t high = COUNT_OF(gauge_table)-1;
	
	for (;;)
	{
		mid  = (low + high) >> 1;

		if (gauge_read_table(mid) < voltage)
		{
			high = mid - 1;
		}
		else if (gauge_read_table(mid+1) > voltage)
		{
			low = mid + 1;
		}
		else
		{
			return mid;
		}
	}
}

#if 0
int main(int argc, char* argv[])
{
	for (uint16_t voltage=2700; voltage< 4200; voltage+=10)
	{
		printf("Voltage=%d -> Capacity=%dmAh\r\n", 
			voltage, gauge_get_capacity(voltage)*10);
	}

	return 0;
}
#endif

