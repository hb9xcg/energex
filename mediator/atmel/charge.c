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
 * @file 	charge.c
 * @brief 	Measures and integrates current.
 * @author 	Markus Walser (markus.walser@gmail.com)
 * @date 	11.02.08
 */

#include <stdlib.h>
#include "charge.h"
#include "adc.h"
#include "io.h"
#include "os_thread.h"
#include "ko.h"


#define R_SHUNT                 5500LL          // [uOhm]
#define R1                      42400ULL        // [mOhm]
#define R2                      1020000ULL      // [mOhm]
#define V_REF                   2500ULL         // [mV]
#define FACTOR_10MA             100ULL
#define SAMPLES_PRE_HOUR        9000000ULL
#define ADC_RESOLUTION          10
#define ADC_MAX_VALUE           (1<<ADC_RESOLUTION)
#define OVER_SAMPLING           (25)
#define OVER_DRIVE		(1)
#define BARREL			151542L 



static uint16_t charge_charged_Ah; 
static uint16_t charge_discharged_Ah; 
static int16_t charge_counter_1mAh;

static int16_t charge_current;
static int16_t charge_adc_sample;
static int8_t  charge_channel;
static int32_t charge_barrel_1mAh;



void charge_setup_channel(void);
void charge_subsample(const int16_t sample);

// gets called each 400us from the timer interrupt
void charge_sample(void)
{
	static int32_t charge_decimation_current;
	
	charge_adc_sample -= adc_get_offset(charge_channel);     // Correct offset of raw sample

	// Create 16 bit dynamics [4.76837uV/LSB];
	switch(charge_channel)
	{
		case CH_CURRENT_1:	// mul 64.0
			charge_adc_sample <<= 6;
			break;
		case CH_CURRENT_10:     // mul 6.4
			charge_adc_sample <<= 5; 
			charge_adc_sample  /= 5;
			break;
		case CH_CURRENT_200:	// div 3.125
			charge_adc_sample *= 8;
			charge_adc_sample /= 25;
			break;
	}


	
	static int8_t idx=1;	
	charge_decimation_current += charge_adc_sample;
	if (++idx >= OVER_SAMPLING)
	{
		idx = 0;
		charge_subsample(charge_decimation_current>>5);
		charge_decimation_current = 0;
	}
	
//	static int8_t ko=0;
//	if (++ko >= 5)
//	{
//		ko = 0;
//		ko_sample(&charge_current_sample);
//	}

	charge_setup_channel();
}

// Gets called every 20ms 16Bit resolution, 1LSB = 1188uA
void charge_subsample(const int16_t sample)
{
	static int16_t counter_charged_Ah;
	static int16_t counter_discharged_Ah;

	charge_current = -sample;

	charge_barrel_1mAh += charge_current;

	if (charge_barrel_1mAh>= BARREL)
	{
		charge_barrel_1mAh -= BARREL;
		charge_counter_1mAh++;
		
		if (++counter_discharged_Ah >= 1000)
		{
			counter_discharged_Ah = 0;
			charge_discharged_Ah++;
		}
	}
	else if (charge_barrel_1mAh <= -BARREL)
	{
		charge_barrel_1mAh += BARREL;
		charge_counter_1mAh--;

		if (++counter_charged_Ah >= 1000)
		{
			counter_charged_Ah = 0;
			charge_charged_Ah++;
		}
	}

//	ko_sample(&charge_current);
}

void charge_setup_channel(void)
{
	int16_t abs_adc_sample = abs(charge_adc_sample);

	if (abs_adc_sample > 3000)	// > 2.6A
	{
		charge_channel = CH_CURRENT_1;
	}
	else if (abs_adc_sample > 115)  // 100mA ... 2.6A
	{
		charge_channel = CH_CURRENT_10;
	}
	else				// < 100mA
	{
		charge_channel = CH_CURRENT_200;
	}
	
	// first 2 dummies to settle down Atmel's analog circuit, then real measurement
	adc_read_int(charge_channel, &charge_adc_sample);  
	adc_read_int(charge_channel, &charge_adc_sample);
	adc_read_int(charge_channel, &charge_adc_sample);
}

void charge_reset(void)
{
	charge_counter_1mAh = 0;
	charge_charged_Ah = 0;
	charge_discharged_Ah = 0;
	charge_barrel_1mAh = 0;
}

void charge_start(void)
{
	charge_counter_1mAh = 0;
}

void charge_set_capacity(int16_t newCapacity)
{
	charge_counter_1mAh = newCapacity*10;
}

int16_t charge_get_capacity(void)
{
	return charge_counter_1mAh/10;
}

void charge_set_total_discharge(uint16_t new_total)
{
	charge_discharged_Ah = new_total;
}

uint16_t charge_get_total_discharge(void)
{
	return charge_discharged_Ah; 
}

void charge_set_total_charge(uint16_t new_total)
{
	charge_charged_Ah = new_total;
}

uint16_t charge_get_total_charge(void)
{
	return charge_charged_Ah; 
}

int32_t charge_get_barrel(void)
{
	return charge_barrel_1mAh;
}

void charge_set_barrel(int32_t barrel)
{
	charge_barrel_1mAh = barrel;
}

// Returns actual current [10mA]
int16_t charge_get_current()
{
	int32_t current;
	
	os_enterCS();
	current = charge_current;	// 16Bit resolution, 1LSB = 1.03mA
	os_exitCS();

	// div 8.419
	current <<= 6;
	current  /= 539;

	return current;
}

