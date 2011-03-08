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
 * @file        ds1307.c
 * @brief       RTC-Treiber
 * @author      Markus Walser (markus.walser@gmail.com)
 * @date        16.10.2010
 */
//#define PC_TEST

#include "ds1307.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "error.h"
#ifndef PC_TEST
#include "i2c.h"
#include "uart.h"
#include <avr/pgmspace.h>
#include "os_thread.h"
#endif


static const uint8_t ds1307_address            = 0xD0;
static const uint8_t ds1307_timekeeper_address = 0x00;


#ifdef DEBUG
    static uint8_t ds1307_debug[64];

    #define ERROR(x)        sprintf((char*)ds1307_debug, "%s:%d error=0x%x\r\n", __FUNCTION__, __LINE__, x); \
                            uart_write(ds1307_debug, strlen((char*)ds1307_debug));
#else
    #define ERROR(x)
#endif

#ifndef PC_TEST
int8_t ds1307_write_time(const ds1307_time_t* time)
{
	uint8_t ret;
	uint8_t buffer[8];

	buffer[0] = ds1307_timekeeper_address;
	for (ret=0; ret<7; ret++)
	{
		buffer[ret+1] = time->timekeeper[ret];
	}

	i2c_write(ds1307_address, buffer, 8);
	ret = i2c_wait();
	if (ret != TW_NO_INFO)
	{
		error(ERROR_DS1307_WRITE);
		return -1;
	}
	return 0;
}

int8_t ds1307_read_time(ds1307_time_t* time)
{
	uint8_t ret;

	i2c_read(ds1307_address, ds1307_timekeeper_address, time->timekeeper, 7);
	ret = i2c_wait();
	if ( ret != TW_NO_INFO) {
		error(ERROR_DS1307_READ);
		return -1;
	}
	return 0;
}
#endif

void ds1307_decode_time(const ds1307_time_t* time, char* output)
{
	sprintf_P(output, PSTR("%d.%d.%d %02d:%02d:%02d"), 
			time->date  + time->date_10  * 10,
			time->month + time->month_10 * 10,
			time->year  + time->year_10  * 10 + 2000,
			
			time->hours    + time->hours_10   * 10,
			time->minutes  + time->minutes_10 * 10,
			time->seconds  + time->seconds_10 * 10);
}

void ds1307_encode_time(const char* input, ds1307_time_t* time)
{
	while( !isdigit(*input))
	{
		input++;
	}

	int date, month, year, hours, minutes, seconds;
	sscanf(input, "%u.%u.%u %u:%u:%u", 
		&date, &month, &year, &hours, &minutes, &seconds);

	year -= 2000;

	time->date     = date  % 10;
	time->date_10  = date  / 10;
	time->month    = month % 10;
	time->month_10 = month / 10;
	time->year     = year  % 10;
	time->year_10  = year  / 10;

	time->seconds    = seconds % 10;
	time->seconds_10 = seconds / 10;
	time->minutes    = minutes % 10;
	time->minutes_10 = minutes / 10;
	time->hours      = hours % 10;
	time->hours_10   = hours / 10;
}

#ifdef PC_TEST
int main(int argc, char* argv[])
{
	char* date = "a 16.10.2010 21:41:52";
	char result[64];

	ds1307_time_t time;

	ds1307_encode_time(date, &time);

	ds1307_decode_time(&time, result);

	puts(result);
}
#endif
