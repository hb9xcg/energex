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



static int64_t charge_capacity;
static int64_t charge_total; 

static int32_t charge_current_oversampling;
static int16_t charge_current_sample;
static int32_t charge_current;

static int8_t  charge_channel;

enum EState
{
	eCharging,
	eDischarging
} eState;

static enum EState charge_state = eCharging;

uint8_t charge_select_channel(void);

// gets called each 400us from the timer interrupt
void charge_sample(void)
{
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
	charge_current_oversampling += charge_current_sample;
	if (++idx >= OVER_SAMPLING)
	{
		idx = 0;
		charge_current = charge_current_oversampling;
		charge_current_oversampling = 0;
		if (charge_current<0)
		{
			// sum up only discharged capacity for Ah counter
			charge_total += -(charge_current>>OVER_SAMPLING_LOG);
		}
	}

	charge_capacity += charge_current_sample;

	charge_channel = charge_select_channel();
	adc_read_int(charge_channel, &charge_current_sample);  // dummy to settle down Atmel's analog circuit
	adc_read_int(charge_channel, &charge_current_sample);  // Real measurement
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
	charge_capacity = 0;
}

void charge_set_capacity(uint16_t newCapacity)
{
	charge_capacity  = newCapacity;
	charge_capacity *= 24503204LL;
}

int16_t charge_get_capacity(void)
{
	int32_t charge10mAh; 

	// 1 Sample * 1 LSB = 1.47uAs
	charge10mAh = charge_capacity / 24503204LL; // [10mAh]

	return charge10mAh;
}

void charge_set_total_discharge(uint16_t new_total)
{
	charge_total  = new_total;
	charge_total *= 76572512ULL;
}

int16_t charge_get_total_discharge(void)
{
	int32_t totalAh; 

	// 1 OverSample = 47uAs
	totalAh = charge_total / 76572512ULL;

	return totalAh; 
}

// Returns actual current [mA]
int16_t charge_get_current()
{
	int32_t current10mA = charge_current;
	
	//current10mA = current10mA * (V_REF * R1 * FACTOR_10MA / R2 / R_SHUNT / ADC_MAX_VALUE); 6625/3264
	
	current10mA  *= 1193L; // 1325 for 5mOhm reduce 10% for wire
	current10mA  /= 3264L;

	// oversampling
	current10mA >>= OVER_SAMPLING_LOG;

	return current10mA;
}
