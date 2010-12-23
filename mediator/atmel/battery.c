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
#include <assert.h>
#include <stdio.h>
#include <avr/io.h>
#include "mediator.h"
#include "battery.h"
#include "sensors.h"
#include "adc.h"
#include "charge.h"
#include "protocol.h"
#include "os_thread.h"
#include "delay.h"
#include "error.h"
#include "data.h"
#include "io.h"
#include "ltc6802.h"


#define ADC_MAX_VALUE		(1<<ADC_RESOLUTION)
#define OVER_SAMPLING_LOG	6
#define OVER_SAMPLING		(1<<OVER_SAMPLING_LOG)
#define V_REF		   	2500ULL      // [mV]
#define ADC_RESOLUTION		9
#define NBR_OF_BATTERIES	3

battery_t battery;

static uint16_t battery_voltage; // [ADC bits]
int8_t battery_enable_sampling = 0;


// gets called every 400us from the timer interrupt 
void battery_sample(void)
{
	static uint8_t idx;
	static uint16_t voltage_oversampling;
	static int16_t voltage_sample;

	static int8_t toggle;

	toggle ^= 1;
	if (toggle)
	{
		charge_sample();
	}
	else
	{
		// Correct offset of raw sample
		voltage_sample -= adc_get_offset(CH_VOLTAGE);
		if (voltage_sample<7)
		{
			voltage_sample = 7;
		}
		voltage_oversampling += voltage_sample;

		if (++idx >= OVER_SAMPLING )
		{
			idx = 0;
			battery_voltage = voltage_oversampling;
			voltage_oversampling = 0;
			voltage_sample = 0;
		}

		adc_read_int( CH_VOLTAGE, &voltage_sample);
		adc_read_int( CH_VOLTAGE, &voltage_sample);
		adc_read_int( CH_VOLTAGE, &voltage_sample);
	}
}

void battery_init(void)
{
	battery.binfo       = 0;
	battery.voltage     = 0;
	battery.current     = 0;

	battery_enable_sampling = 1;
}

uint16_t battery_get_voltage(void)
{
	uint32_t voltage32;

	voltage32   = battery_voltage;
	voltage32  *= data_voltage_calibration;

	voltage32 >>= (OVER_SAMPLING_LOG + ADC_RESOLUTION);
	voltage32  -= 500;

	return voltage32;
}

void battery_calibrate(uint16_t targetVoltage)
{
	uint16_t currentVoltage = battery_get_voltage();

	currentVoltage += 500;
	targetVoltage  += 500;

	uint32_t new_calibration = targetVoltage;

        new_calibration	*= data_voltage_calibration;
	new_calibration /= currentVoltage;

	data_voltage_calibration = new_calibration;
}

uint16_t battery_get_power(void)
{
	int16_t current, voltage;
	int32_t power;

	current = charge_get_current();
	voltage = battery_get_voltage();

	power = current * voltage;
	power /= 30000;

	return power;
}
/*
int16_t battery_get_temperature(void)
{
	uint8_t idx;
	uint16_t average = 0;
	int32_t temperature32;
	
	for (idx=0; idx<OVER_SAMPLING; idx++)
	{
		average += temperature[idx];
	}

	temperature32   = average;
	temperature32  *= (V_REF*10);
	temperature32 >>= (OVER_SAMPLING_LOG + ADC_RESOLUTION);
	temperature32  -= 6000;

	return temperature32;
}
*/
void battery_command(uint16_t relais_state)
{
	if (relais_state) {
		ePowerSoll = ePowerFull;
	} else {
		ePowerSoll = ePowerSave;
	}
}

uint8_t battery_get_index(uint8_t address)
{
	if (address < BATTERY_1 ||
            address > BATTERY_3 )
        {
                error(ERROR_INVALID_ADDRESS);
        }

	return address - BATTERY_1;
}

void battery_set_parameter_value(uint8_t parameter, uint8_t address, uint16_t value)
{
	uint8_t index;

	os_enterCS();

	switch(parameter)
	{
	case DRIVE_STATE:	mediator_set_drive_state(value);
				break;
	case COMMAND:		battery_command(value);
				break;
	case BUS_ADRESSE:	battery.address	= value;
				break;
	case TOTAL_SPANNUNG:	battery.voltage	= value;
				break;
	case AH_ZAEHLER:	charge_set_capacity(value);
				break;
	case SYM_SPANNUNG:	index = battery_get_index(address);
				battery.sym_voltage[index] = value;
				break;
	case BATTERIE_TEMP:	index = battery_get_index(address);
				battery.temperature[index] = value;
				break;
	case GESCHWINDIGKEIT:
	case TAGESKILOMETER:
				break;
	default:
				error(ERROR_SET_UNKNOWN_PARAMETER);
				break;
	}

	os_exitCS();
}

int16_t battery_get_parameter_value(uint8_t parameter, uint8_t address)
{
	uint16_t value;
	
	if (address < BATTERY_1 ||
	    address > BATTERY_3 )
	{
		error(ERROR_INVALID_ADDRESS);
	}
	
	address -= BATTERY_1;

	os_enterCS();

	switch(parameter)
	{						
	case MODEL_TYPE:		value = 1;			break;
	case PROGRAM_REV:		value = 530;			break;
	case PAR_TAB_REV:		value = 530;			break;
	
	case NENNSPNG:			value = 35520;			break;
	case NENNSTROM:			value = 2500;			break;
	case SERIE_NUMMER:		value = 11178;			break;
	case REP_DATUM: 		value = 0;			break;
	case STANDZEIT:			value = 64;			break;
	case FAHR_LADE_ZEIT:		value = 9;			break;
	case LAST_ERROR:		value = 0;			break;
	case BUS_ADRESSE:		value = battery.address;	break;

	case DRIVE_STATE:		value = mediator_get_drive_state();	break;
	case COMMAND:			value = !battery_info_get(BAT_REL_OPEN);	break;
	case PARAM_PROT:		value = 0;			break;

	case BINFO:			value = battery_get_info();	break;
	
	// We simulate 3 batteries. Thus each of them reports one third of the real value.
	case IST_STROM:			value = charge_get_current()/NBR_OF_BATTERIES;	break;
	case LADESTROM:			value = 400;			break;
	case FAHRSTROM:			value = -800;			break;
	case TOTAL_SPANNUNG:		value = battery_get_voltage();	break;
	case SOLL_LADESPG:		value =	40000;			break;
	
	 // We simulate 3 batteries. Thus each of them reports one third of the real value.
	case AH_ZAEHLER:		value = charge_get_capacity() /
						NBR_OF_BATTERIES;	break;
	case Q:				value = -1400;				break;
	case LEISTUNG:			value = battery_get_power();		break;
	case BATTERIE_TEMP:		value = battery.temperature[address];	break;
	case FINFO:			value = 0;				break;
	case SYM_SPANNUNG:		value = battery.sym_voltage[address];	break;
				
	case TEILSPANNUNG1:		value = 5100;	break;
	case TEILSPANNUNG2:		value = 5100;	break;
	case TEILSPANNUNG3:		value = 5100;	break;
	case TEILSPANNUNG4:		value = 5100;	break;
	case TEILSPANNUNG5:		value = 5100;	break;
	case TEILSPANNUNG6:		value = 5100;	break;
	case TEILSPANNUNG7:		value = 5100;	break;
			
	case TEMPERATUR1:		value = ltc_get_external_temperature(0);  break;
	case TEMPERATUR2:		value = ltc_get_external_temperature(1);  break;
	case TEMPERATUR3:		value = ltc_get_external_temperature(2);  break;
	case TEMPERATUR4:		value = ltc_get_external_temperature(3);  break;
	case TEMPERATUR5:		value = ltc_get_external_temperature(4);  break;
	case TEMPERATUR6:		value = ltc_get_external_temperature(5);  break;
	case TEMPERATUR7:		value = ltc_get_external_temperature(6);  break;
	case TEMPERATUR8:		value = ltc_get_external_temperature(7);  break;
	case TEMPERATUR9:		value = ltc_get_external_temperature(8);  break;
	case TEMPERATUR10:		value = ltc_get_external_temperature(9);  break;
	case TEMPERATUR11:		value = ltc_get_external_temperature(10);  break;
	case TEMPERATUR12:		value = ltc_get_external_temperature(11);  break;
	case TEMPERATUR13:		value = ltc_get_external_temperature(12);  break;
	case TEMPERATUR14:		value = ltc_get_external_temperature(13);  break;
	
	case PC_CALIBR_TEMP:		value = 0x5678;			break;
	case MAX_BAT_TEMP:		sensors_get_max_temperatur((int16_t*)&value);	break;
	case UMGEBUNGS_TEMP:		value = mediator_get_temperature();		break;
	case MAX_LADETEMP:		value = 4500;			break;
	case MIN_LADETEMP:		value = 0;			break;
	case MAX_FAHRTEMP:		value = 4500;			break;
	case MIN_FAHRTEMP:		value = 0;			break;
	case MAX_LAGERTEMP:		value = 4500;			break;
	case MIN_LAGERTEMP:		value = 0;			break;

	case MAX_KAPAZITAET:		value = 305;	break;
	case MIN_KAPAZITAET:		value = 280;	break;
	case GELADENE_AH:		value = charge_get_total_charge()/NBR_OF_BATTERIES;	break;
	case ENTLADENE_AH:		value = charge_get_total_discharge()/NBR_OF_BATTERIES;	break;
	case LADEZYKLEN:		value = data_charge_cycles;		break;
	case TIEFENTLADE_ZYKLEN:	value = data_deep_discharge_cycles;	break;
	
	case MAX_ENTLADE_STROM:		value = -800;	break; // Times 3 blocks -> 24A
				
	case ZYKLUS_UEBER_110:		value = data_stat.cycles_over_110;	break;
	case ZYKLUS_UEBER_100:		value = data_stat.cycles_over_100;	break;
	case ZYKLUS_UEBER_90:		value = data_stat.cycles_over_90;	break;
	case ZYKLUS_UEBER_80:		value = data_stat.cycles_over_80;	break;
	case ZYKLUS_UEBER_70:		value = data_stat.cycles_over_70;	break;
	case ZYKLUS_UEBER_60:		value = data_stat.cycles_over_60;	break;
	case ZYKLUS_UEBER_50:		value = data_stat.cycles_over_50;	break;
	case ZYKLUS_UEBER_40:		value = data_stat.cycles_over_40;	break;
	case ZYKLUS_UEBER_30:		value = data_stat.cycles_over_30;	break;
	case ZYKLUS_UEBER_20:		value = data_stat.cycles_over_20;	break;
	case ZYKLUS_UEBER_10:		value = data_stat.cycles_over_10;	break;
	case ZYKLUS_UNTER_10:		value = data_stat.cycles_under_10;	break;

	case MAX_UEBERLADUNG:		value = 850;	break;
	case MIN_LDG_F_VOLL:		value = -500;	break;
	case STROM_ZUNAHME:		value = 40;	break;
	case MAX_LADE_SPG:		value = 40000;	break;
	case MIN_LADE_TEMP:		value = 0;	break;
	case MAX_LADE_TEMP:		value = 4500;	break;
	case MAX_TEMP_ZUNAHME:		value = 100;	break;
	case MAX_LADEZEIT:		value = 800;	break;
	case SYMMETRIER_STROM:		value = 28;	break;

	case LADE_STR_UNTER_M10:	value = 56;	break;
	case LADE_STR_UEBER_M10:	value = 56;	break;
	case LADE_STR_UEBER_P00:	value = 280;	break;
	case LADE_STR_UEBER_P10:	value = 280;	break;
	case LADE_STR_UEBER_P20:	value = 280;	break;
	case LADE_STR_UEBER_P30:	value = 280;	break;
	case LADE_STR_UEBER_P40:	value = 79;	break;
	case LADE_STR_UEBER_P45:	value = 28;	break;
	case LADE_STR_UEBER_P50:	value = 28;	break;
			
	case LADE_SPG_UNTER_M10:	value = 40000;	break;
	case LADE_SPG_UEBER_M10:	value = 40000;	break;
	case LADE_SPG_UEBER_P00:	value = 40000;	break;
	case LADE_SPG_UEBER_P10:	value = 40000;	break;
	case LADE_SPG_UEBER_P20:	value = 40000;	break;
	case LADE_SPG_UEBER_P30:	value = 40000;	break;
	case LADE_SPG_UEBER_P40:	value = 37440;	break;
	case LADE_SPG_UEBER_P45:	value = 36480;	break;
	case LADE_SPG_UEBER_P50:	value = 35520;	break;
			
	case NOM_KAPAZITAET:		value = data_nominal_capacity/3;break;
	case MIN_FAHR_SPANNUNG:		value = 31000;	break;
	case SELBST_ENTL_STROM:		value = 28000;	break;
	case TIEFENTLADE_SPG:		value = TIEFENTLADE_SPANNUNG;	break;
	case MAX_SPANNUNG_DIFF:		value = 500;	break;
	case MIN_FAHR_TEMP_B:		value = -2500;	break;
	case MAX_FAHR_TEMP_B:		value = 6000;	break;
	case MAX_FAHR_STROM:		value = -1000;	break;
	
	case AD_STROM:			value = 0x817F;	break;
	
	case KAL_TEMP_7_6:		value = 0xFFFF;	break;
	case KAL_TEMP_5_4:		value = 0xFEFD;	break;
	case KAL_TEMP_3_2:		value = 0xFCFD;	break;
	case KAL_TEMP_1_AMB:		value = 0x00FE;	break;
	case KAL_TEMP_GD_14:		value = 0x7C00;	break;
	case KAL_TEMP_13_12:		value = 0x00FF;	break;
	case KAL_TEMP_11_10:		value = 0xFE00;	break;
	case KAL_TEMP_9_8:		value = 0x0002;	break;

	case SENSOR_MASK:		value = 0x3F00;	break;
			
	case OFFS_KLEIN_STL:		value = 1508;	break;
	case OFFS_GROSS_STL:		value = 12523;	break;
	case KALIB_SPG_1:		value = 0x00A3;	break;
	case KALIB_SPG_2:		value = 0x009F;	break;
	case KALIB_SPG_3:		value = 0x00A6;	break;
	case KALIB_SPG_4:		value = 0x00B1;	break;
	case KALIB_SPG_5:		value = 0x007C;	break;
	case KALIB_SPG_6:		value = 0x00A7;	break;
	case KALIB_SPG_9:		value = 0x00A7;	break;
			
	case DEBUG_VALUE_C:		value = 22498;	break;
	case DEBUG_VALUE_H:		value = 0x57E2;	break;
	case DEBUG_VALUE_ADDR:		value = 0xFFFF;	break;

	case GESCHWINDIGKEIT:		value = 0;	break;
	case TAGESKILOMETER:		value = 0;	break;
	
	default:
		value = 0x0000;
		error(ERROR_GET_UNKNOWN_PARAMETER);
	}

	os_exitCS();

	return value;
}

void battery_info_set(uint8_t bitNo)
{
	os_enterCS();
	battery.binfo |= (1<<bitNo);
	os_exitCS();
}

void battery_info_clear(uint8_t bitNo)
{
	os_enterCS();
	battery.binfo &= ~(1<<bitNo);
	os_exitCS();
}

uint8_t battery_info_get(uint8_t bitNo)
{
	uint8_t result;

	os_enterCS();
	result = (battery.binfo & (1<<bitNo)) != 0;
	os_exitCS();

	return result;
}

uint16_t battery_get_info(void)
{
	uint16_t result;

	os_enterCS();
	result = battery.binfo;
	os_exitCS();

	return result;
}

