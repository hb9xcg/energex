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
#include <assert.h>
#include <stdio.h>
#include <avr/io.h>
#include "plc.h"
#include "battery.h"
#include "adc.h"
#include "protocol.h"
#include "os_thread.h"
#include "delay.h"
#include "error.h"
#include "data.h"


#define VOLTAGE_INV_R1		660000ULL    // [Ohm]
#define VOLTAGE_INV_R2		3740ULL      // [Ohm]
#define V_REF		   	2500ULL      // [mV]
#define ADC_RESOLUTION		9
#define ADC_MAX_VALUE		(1<<ADC_RESOLUTION)

#define NBR_OF_BATTERIES	3

battery_t battery;

void battery_init(void)
{
	battery.binfo       = 0;
	battery.voltage     = 0;
	battery.current     = 0;
	battery.ah_counter  = 0;
}

void battery_set_parameter_value(uint8_t parameter, uint16_t value)
{
	os_enterCS();

	switch(parameter)
	{
	case DRIVE_STATE:	plc_set_drive_state(value);	break;
	case COMMAND:						break;
	case BUS_ADRESSE:	battery.address 	= value;break;
	case TOTAL_SPANNUNG:	battery.voltage 	= value;break;
	case AH_ZAEHLER:	battery.ah_counter      = value;break;
	case TEMPERATUR1:	battery.temperatur_1  = value;	break;
	case TEMPERATUR2:	battery.temperatur_2  = value;	break;
	case TEMPERATUR3:	battery.temperatur_3  = value;	break;
	case TEMPERATUR4:	battery.temperatur_4  = value;	break;
	case TEMPERATUR5:	battery.temperatur_5  = value;	break;
	case TEMPERATUR6:	battery.temperatur_6  = value;	break;
	case TEMPERATUR7:	battery.temperatur_7  = value;	break;
	case TEMPERATUR8:	battery.temperatur_8  = value;	break;
	case TEMPERATUR9:	battery.temperatur_9  = value;	break;
	case TEMPERATUR10:	battery.temperatur_10 = value;	break;
	case TEMPERATUR11:	battery.temperatur_11 = value;	break;
	case TEMPERATUR12:	battery.temperatur_12 = value;	break;
	case TEMPERATUR13:	battery.temperatur_13 = value;	break;
	case TEMPERATUR14:	battery.temperatur_14 = value;	break;
	case GESCHWINDIGKEIT:
	case TAGESKILOMETER:					break;
	default:						break;
	}

	os_exitCS();
}

int16_t battery_get_parameter_value(uint8_t parameter)
{
	uint16_t value;

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

	case DRIVE_STATE:		value = plc_get_drive_state();	break;
	case COMMAND:			value = !battery_info_get(BAT_REL_OPEN);	break;
	case PARAM_PROT:		value = 0;					break;

	case BINFO:			value = battery_get_info();	break;
	
	// We simulate 3 batteries. Thus each of them reports one third of the real value.
	case IST_STROM:			value = 0;	break;
	case LADESTROM:			value = 400;			break;
	case FAHRSTROM:			value = -800;			break;
	case TOTAL_SPANNUNG:		value = 0;			break;
	case SOLL_LADESPG:		value =	40000;			break;
	
	 // We simulate 3 batteries. Thus each of them reports one third of the real value.
	case AH_ZAEHLER:		value = battery.ah_counter;	break;
	case Q:				value = -1400;					break;
	case LEISTUNG:			value = 0;			break;
	case BATTERIE_TEMP:		value = 0;	break;
	case FINFO:			value = 0;					break;
	case SYM_SPANNUNG:		value = 26;					break;
				
	case TEILSPANNUNG1:		value = 5100;	break;
	case TEILSPANNUNG2:		value = 5100;	break;
	case TEILSPANNUNG3:		value = 5100;	break;
	case TEILSPANNUNG4:		value = 5100;	break;
	case TEILSPANNUNG5:		value = 5100;	break;
	case TEILSPANNUNG6:		value = 5100;	break;
	case TEILSPANNUNG7:		value = 5100;	break;
	
	case TEMPERATUR1:		value = battery.temperatur_1;	break;
	case TEMPERATUR2:		value = battery.temperatur_2;	break;
	case TEMPERATUR3:		value = battery.temperatur_3;	break;
	case TEMPERATUR4:		value = battery.temperatur_4;	break;
	case TEMPERATUR5:		value = battery.temperatur_5;	break;
	case TEMPERATUR6:		value = battery.temperatur_6;	break;
	case TEMPERATUR7:		value = battery.temperatur_7;	break;
	case TEMPERATUR8:		value = battery.temperatur_8;	break;
	case TEMPERATUR9:		value = battery.temperatur_9;	break;
	case TEMPERATUR10:		value = battery.temperatur_10;	break;
	case TEMPERATUR11:		value = battery.temperatur_11;	break;
	case TEMPERATUR12:		value = battery.temperatur_12;	break;
	case TEMPERATUR13:		value = battery.temperatur_13;	break;
	case TEMPERATUR14:		value = battery.temperatur_14;	break;
			
	case PC_CALIBR_TEMP:		value = 0x5678;			break;
	case MAX_BAT_TEMP:		value = 0;	break;
	case UMGEBUNGS_TEMP:		value = 0;		break;
	case MAX_LADETEMP:		value = 4500;			break;
	case MIN_LADETEMP:		value = 0;			break;
	case MAX_FAHRTEMP:		value = 4500;			break;
	case MIN_FAHRTEMP:		value = 0;			break;
	case MAX_LAGERTEMP:		value = 4500;			break;
	case MIN_LAGERTEMP:		value = 0;			break;

	case MAX_KAPAZITAET:		value = 305;	break;
	case MIN_KAPAZITAET:		value = 280;	break;
	case GELADENE_AH:		value = 0;	break;
	case ENTLADENE_AH:		value = 0;	break;
	case LADEZYKLEN:		value = 0;		break;
	case TIEFENTLADE_ZYKLEN:	value = 0;	break;
	
	case MAX_ENTLADE_STROM:		value = -800;	break; // Times 3 blocks -> 24A
				
	case ZYKLUS_UEBER_110:		value = 0;	break;
	case ZYKLUS_UEBER_100:		value = 0;	break;
	case ZYKLUS_UEBER_90:		value = 0;	break;
	case ZYKLUS_UEBER_80:		value = 0;	break;
	case ZYKLUS_UEBER_70:		value = 0;	break;
	case ZYKLUS_UEBER_60:		value = 0;	break;
	case ZYKLUS_UEBER_50:		value = 0;	break;
	case ZYKLUS_UEBER_40:		value = 0;	break;
	case ZYKLUS_UEBER_30:		value = 0;	break;
	case ZYKLUS_UEBER_20:		value = 0;	break;
	case ZYKLUS_UEBER_10:		value = 0;	break;
	case ZYKLUS_UNTER_10:		value = 0;	break;

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
			
	case NOM_KAPAZITAET:		value = 3;	break;
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

	case SENSOR_MASK:		value = 0x0000;	break;
			
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
		error(ERROR_UNKNOWN_PARAMETER);
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

void battery_set_info(uint16_t binfo)
{
	os_enterCS();
	battery.binfo = binfo;
	os_exitCS();
}

void battery_set_current(uint16_t current)
{
	os_enterCS();
	battery.current = current;
	os_exitCS();
}

void battery_set_voltage(uint16_t voltage)
{
	os_enterCS();
	battery.voltage = voltage;
	os_exitCS();
}

int16_t battery_get_current(void)
{
	int16_t current;

	os_enterCS();
	current = battery.current;
	os_exitCS();

	return current;
}

uint16_t battery_get_voltage(void)
{
	uint16_t voltage;

	os_enterCS();
	voltage = battery.voltage;
	os_exitCS();

	return voltage;
}

