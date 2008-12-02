/*
 * Energex
 * 
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your
 * option) any later version. 
 * This program is distributed in the hope that it will be 
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
 * PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public 
 * License along with this program; if not, write to the Free 
 * Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307, USA.
 * 
 */

/*! 
 * @file 	charge.c
 * @brief 	Measures and integrates current.
 * @author 	Markus Walser (markus.walser@gmail.com)
 * @date 	11.02.08
 */

#include "charge.h"
#include "adc.h"


static int16_t charge_current;
static int64_t charge_capacity;

#define OVER_SAMPLING_LOG	4
#define OVER_SAMPLING 		(1<<OVER_SAMPLING_LOG)

static int16_t charge_charging_sample;
static int16_t charge_discharging_sample;

enum EState
{
	eCharging,
	eDischarging
} eState;

// gets called each 400us from the timer interrupt
void charge_sample(void)
{
	static enum EState eState = eCharging;

	if( charge_charging_sample > charge_discharging_sample )
	{
		charge_current = charge_charging_sample; // save by ISR completed result
		adc_read_int( CH_CHARGE, &charge_charging_sample);
		charge_discharging_sample = 0;
	}
	else if( charge_charging_sample < charge_discharging_sample )
	{
		charge_current = -charge_discharging_sample; // save by ISR completed result
		adc_read_int( CH_DISCHARGE, &charge_discharging_sample);
		charge_charging_sample = 0;	
	}
	else
	{
		switch(eState)
		{
			case eCharging:
				eState = eDischarging;
				charge_current = charge_charging_sample;
				adc_read_int( CH_DISCHARGE, &charge_discharging_sample);
				break;
			case eDischarging:
				eState = eCharging;
				charge_current = -charge_discharging_sample;
				adc_read_int(CH_CHARGE, &charge_charging_sample);
				break;
		}
	}
	charge_capacity -= charge_current;
}

void charge_reset(void)
{
	charge_capacity = 0;
}

void charge_set_capacity(uint16_t newCapacity)
{
	charge_capacity  = newCapacity;
	charge_capacity *= 3072000;
}

int16_t charge_get_capacity(void)
{
	int32_t charge10mAh; 

#if 0
	capacity = charge_capacity / 1024; // 10Bit AD converter
	capacity *= 2500; // Reference voltage [mV]
	capacity *= 12;   // Calculate differential amplifier
	capacity /= 2500; // 400us -> s	
	capacity /= 3600; // s -> h, Now we have mAh
#else
	charge10mAh = charge_capacity / 3072000; // [10mAh]
#endif
	return charge10mAh;
}

// Returns actual current [mA]
int16_t charge_get_current()
{
	int32_t current10mA;

	current10mA = charge_current;
#if 0
	current *= 2500; // Reference voltage [mV]
	current *= 12;   // Calculate differential amplifier
	current /= 1024; // 10Bit AD converter
#else
	current10mA *= 4;
#endif
	return current10mA;
}