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

#ifndef DS1307_H_DEF
#define DS1307_H_DEF

#include "global.h"

/*! 
 * @file        ds1307.h
 * @brief       RTC-Treiber
 * @author      Markus Walser (markus.walser@gmail.com)
 * @date        16.10.2010
 */


typedef union 
{
	struct 
	{
		uint8_t seconds:4;
		uint8_t seconds_10:3;
		uint8_t CH:1;
		uint8_t minutes:4;
		uint8_t minutes_10:4;
		uint8_t hours:4;
		uint8_t hours_10:2;
		uint8_t format:2;
		uint8_t day:8;
		uint8_t date:4;
		uint8_t date_10:4;
		uint8_t month:4;
		uint8_t month_10:4;
		uint8_t year:4;
		uint8_t year_10:4;
	} __attribute__((packed));
	uint8_t timekeeper[7];
} ds1307_time_t;

int8_t ds1307_read_time(ds1307_time_t* time);
int8_t ds1307_write_time(const ds1307_time_t* time);
void   ds1307_decode_time(const ds1307_time_t* time, char* output);
void   ds1307_encode_time(const char* input, ds1307_time_t* time);

#endif
