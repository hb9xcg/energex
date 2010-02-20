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
//#define BARREL		612580L // 42.2 Ohm
#define BARREL			366601L // 68 Ohm



static uint16_t charge_charged_Ah; 
static uint16_t charge_discharged_Ah; 
static int16_t charge_counter_1mAh;

static int16_t charge_current_sample;
static int16_t charge_current;

static int8_t  charge_channel;

enum EState
{
	eCharging,
	eDischarging
} eState;

static enum EState charge_state = eCharging;

uint8_t charge_select_channel(void);
void charge_subsample(const int16_t sample);

// gets called each 400us from the timer interrupt
void charge_sample(void)
{
	static int32_t decimation_current;
	
	charge_current_sample -= adc_get_offset(charge_channel);     // Correct offset of raw sample
	charge_current_sample *= adc_get_resolution(charge_channel); // [488uV/LSB];

	if (charge_current_sample > 0)
	{
		charge_state = eCharging;
	}
	else if (charge_current_sample < 0)
	{
		charge_state = eDischarging;
	}
	else
	{
		charge_state = (charge_state==eCharging) ? eDischarging : eCharging;
	}
	

	static int8_t idx=1;	
	decimation_current += charge_current_sample;
	if (++idx >= OVER_SAMPLING)
	{
		idx = 0;
		charge_subsample(decimation_current>>2);
		decimation_current = 0;
	}
	
//	static int8_t ko=0;
//	if (++ko >= 5)
//	{
//		ko = 0;
//		ko_sample(&charge_current_sample);
//	}

	charge_channel = charge_select_channel();
	adc_read_int(charge_channel, &charge_current_sample);  // dummy to settle down Atmel's analog circuit
	adc_read_int(charge_channel, &charge_current_sample);  // Real measurement
}

// Gets called every 10ms 16Bit resolution, 1LSB = 587.678uA -> 982uA
void charge_subsample(const int16_t sample)
{
	static int32_t barrel_1mAh;
	static int16_t counter_charged_Ah;
	static int16_t counter_discharged_Ah;

	charge_current = sample;

	barrel_1mAh += charge_current;

	if (barrel_1mAh>= BARREL)
	{
		barrel_1mAh -= BARREL;
		charge_counter_1mAh++;
		
		if (++counter_discharged_Ah >= 1000)
		{
			counter_discharged_Ah = 0;
			charge_discharged_Ah++;
		}
	}
	else if (barrel_1mAh <= -BARREL)
	{
		barrel_1mAh += BARREL;
		charge_counter_1mAh--;

		if (++counter_charged_Ah >= 1000)
		{
			counter_charged_Ah = 0;
			charge_charged_Ah++;
		}
	}

//	ko_sample(&charge_current);
}

uint8_t charge_select_channel(void)
{
	static int8_t hysteresis = -35; // = 0.13A
	uint8_t channel;

	// charge_current_sample [488uV/LSB] (3.67mA/LSB) and limit at input 505 (=> 1.85A)

	if (abs(charge_current_sample) < 470+hysteresis)
	{
		// if current is below ~1.7A we switch to differential channels
		hysteresis = 35;
		channel = (charge_state==eCharging) ? CH_CHARGE_10 : CH_DISCHARGE_10;
	}
	else
	{
		// if current is above 2A we switch to single sided channels
		hysteresis = -35;
		channel = (charge_state==eCharging) ? CH_CHARGE_1 : CH_DISCHARGE_1;
	}
	
	return channel;
}

void charge_reset(void)
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

uint16_t charge_get_total_charge(void)
{
	return charge_charged_Ah; 
}

// Returns actual current [10mA]
int16_t charge_get_current()
{
	int16_t current;
	
	os_enterCS();
	current = charge_current;
	os_exitCS();

//	return current/17;	// 42.2 Ohm

	return current/10 - current/400;	// 68 Ohm
}

