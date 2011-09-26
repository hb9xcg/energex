/***************************************************************************
 *   Energex                                                               *
 *                                                                         *
 *   Copyright (C) 2008-2009 by Markus Walser                              *
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
 * @file 	ko.c
 * @brief 	Measures and integrates current.
 * @author 	Markus Walser (markus.walser@gmail.com)
 * @date 	4.12.09
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include <math.h>
#include <avr/pgmspace.h>
#include "ko.h"
#include "uart.h"
#include "mediator.h"


#define R_SHUNT                 5500LL          // [uOhm]
#define R1                      42400ULL        // [mOhm]
#define R2                      1020000ULL      // [mOhm]
#define V_REF                   2500ULL         // [mV]
#define FACTOR_10MA             100ULL
#define SAMPLES_PRE_HOUR        9000000ULL
#define ADC_RESOLUTION          10
#define ADC_MAX_VALUE           (1<<ADC_RESOLUTION)
#define OVER_SAMPLING_LOG       5
#define OVER_SAMPLING           (1<<OVER_SAMPLING_LOG)
#define OVER_DRIVE		(1)

#define BUFFER_SIZE             100
#define SWAP(A,B)		{     				\
					ko_temp = (A);		\
					(A) = (B);		\
					(B) = ko_temp;		\
				}


static int16_t  ko_samples[3][BUFFER_SIZE];
static int8_t   ko_sample_idx;
static int16_t  ko_maximum;
static int16_t  ko_minimum;
static int8_t   ko_rec_max;
static int8_t   ko_rec_min;
static int8_t   ko_max_center;
static int8_t   ko_min_center;
static int8_t   ko_temp;

static int8_t   ko_rec_buffer_idx = 0;
static int8_t   ko_max_buffer_idx = 1;
static int8_t   ko_min_buffer_idx = 2;


void ko_sample(int16_t* sample)
{
	ko_samples[ko_rec_buffer_idx][ko_sample_idx] = *sample;

	ko_sample_idx++;
	
	if (ko_sample_idx>=BUFFER_SIZE)
	{
		ko_sample_idx = 0;
	}

	if (*sample > ko_maximum && ko_rec_min==0)
	{
		ko_maximum = *sample;
		ko_rec_max = BUFFER_SIZE/2;
		ko_max_center = ko_sample_idx;
	}
	else if (*sample < ko_minimum && ko_rec_max==0)
	{
		ko_minimum = *sample;
		ko_rec_min = BUFFER_SIZE/2;
		ko_min_center = ko_sample_idx;
	}

	if (ko_rec_max>0)
	{
		ko_rec_max--;
		if (ko_rec_max == 0)
		{
			SWAP( ko_rec_buffer_idx, ko_max_buffer_idx);
		}
	}
	else if (ko_rec_min>0)
	{
		ko_rec_min--;
		if (ko_rec_min == 0)
		{
			SWAP( ko_rec_buffer_idx, ko_min_buffer_idx);
		}
	}
}


void ko_print()
{
	int8_t idx, count;
	static char line[64];

	sprintf_P(line, PSTR("Maximum current:\n\r"));
	uart_write( (uint8_t*)line, strlen(line) );

	idx = ko_max_center-(BUFFER_SIZE/2);
	if (idx < 0)
	{
		idx += BUFFER_SIZE;
	}
	for(count=0; count<BUFFER_SIZE; count++)
	{
		sprintf(line, "%d\t%d\r\n", 
			count, ko_samples[ko_max_buffer_idx][idx]);
		uart_write( (uint8_t*)line, strlen(line) );
		if (++idx>=BUFFER_SIZE)
		{
			idx = 0;
		}
	}
	
	sprintf_P(line, PSTR("\n\rMinimum current:\n\r"));
	uart_write( (uint8_t*)line, strlen(line) );
	idx = ko_min_center-(BUFFER_SIZE/2);
	if (idx < 0)
	{
		idx += BUFFER_SIZE;
	}
	for(count=0; count<BUFFER_SIZE; count++)
	{
		sprintf(line, "%d\t%d\r\n", 
			count, ko_samples[ko_min_buffer_idx][idx]);
		uart_write( (uint8_t*)line, strlen(line) );
		if (++idx>=BUFFER_SIZE)
		{
			idx = 0;
		}
	}
}

#if 0
int main(int argc, char* argv[])
{
	int idx;
	for (idx=0; idx<2000; idx++)
	{
		int16_t sample = sin(idx)*32000.0;
		ko_sample(&sample);
		printf("idx=%d, value=%d\n",idx, sample);
	}

	ko_print();

	return 0;
}
#endif
