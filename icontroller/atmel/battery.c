/***************************************************************************
 *   Copyright (C) 2008 by Markus Walser                                   *
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
#include "battery.h"
#include "adc.h"
#include "charge.h"
#include "protocol.h"
#include "os_thread.h"
#include <assert.h>
#include <stdio.h>

#define OVER_SAMPLING_LOG  4
#define OVER_SAMPLING	  (1<<OVER_SAMPLING_LOG)
#define VOLTAGE_INV_R1	   3*346500  // [Ohm]
#define VOLTAGE_INV_R2	   6800     // [Ohm]
#define V_REF		   2510     // [mV]
#define ADC_RESOLUTION     10
#define ADC_MAX_VALUE	   (1<<ADC_RESOLUTION)

battery_t battery;
int16 voltage[OVER_SAMPLING];
int16 temperature[OVER_SAMPLING];

// gets called each 400us from the timer interrupt 
void battery_sample(void)
{
	static uint8_t idx;

	charge_sample();
	adc_read_int( CH_VOLTAGE, &voltage[idx]);
	adc_read_int( CH_TEMPERATURE, &temperature[idx]);
	
	if (++idx == OVER_SAMPLING )
	{
		idx = 0;
	}
}

uint16_t getVoltage(void)
{
	uint8_t idx;
	uint16_t average = 0;
	uint32_t voltage32;
	
	for (idx=0; idx<OVER_SAMPLING; idx++)
	{
		average += voltage[idx];
	}

	voltage32   = average;
	voltage32  *= (V_REF/10 * VOLTAGE_INV_R1 / VOLTAGE_INV_R2 );     // voltage32 *= 35870;
	voltage32 >>= (OVER_SAMPLING_LOG + ADC_RESOLUTION);

	return voltage32;
}

int16_t getTemperature(void)
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

void setParameterValue(uint8_t parameter, uint16_t value)
{
	os_enterCS();

	switch(parameter)
	{
	case DRIVE_STATE:	battery.drive_state  	= value;	break;
	case RELAIS_STATE:	battery.relais_state 	= value;	break;
	case BUS_ADRESSE:	battery.address 	= value;	break;
	case TOTAL_SPANNUNG:	battery.voltage 	= value;	break;
	case AH_ZAEHLER:	charge_set_capacity(value*3);		break;

	default:
		break;
	}

	os_exitCS();
}

int16_t getParameterValue(uint8_t parameter)
{
	uint16_t value;

	os_enterCS();

	switch(parameter)
	{						
	case MODEL_TYPE:		value = 1;			break;
	case PROGRAM_REV:		value = 530;			break;
	case PAR_TAB_REV:		value = 530;			break;
	
	case NENNSPNG:			value = 33600;			break;
	case NENNSTROM:			value = 280;			break;
	case SERIE_NUMMER:		value = 40847;			break;
	case REP_DATUM: 		value = 2;			break;
	case STANDZEIT:			value = 64;			break;
	case FAHR_LADE_ZEIT:		value = 9;			break;
	case LAST_ERROR:		value = 0;			break;
	case BUS_ADRESSE:		value = battery.address;	break;

	case DRIVE_STATE:		value = battery.drive_state;	break;
	case RELAIS_STATE:		value = battery.relais_state;	break;
	case PARAM_PROT:		value = 0;			break;

	case BINFO:			value = battery.binfo;		break;
	
	// We simulate 3 batteries. Thus each of them reports one third of the real value.
	case IST_STROM:			value = charge_get_current()/3;	break;
	case LADESTROM:			value = 280;			break;
	case FAHRSTROM:			value = -1000;			break;
	case TOTAL_SPANNUNG:		value = getVoltage();		break;
	case SOLL_LADESPG:		value =	44000;			break;
	
	 // We simulate 3 batteries. Thus each of them reports one third of the real value.
	case AH_ZAEHLER:		value = charge_get_capacity()/3;			break;
	case Q:				value = -1400;						break;
	case LEISTUNG:			value = (battery.current/100)*(battery.voltage/100);	break;
	case BATTERIE_TEMP:		value = 2000;						break;
	case FINFO:			value = 0;						break;
	case SYM_SPANNUNG:		value = 26;						break;
				
	case TEILSPANNUNG1:		value = 5100;	break;
	case TEILSPANNUNG2:		value = 5100;	break;
	case TEILSPANNUNG3:		value = 5100;	break;
	case TEILSPANNUNG4:		value = 5100;	break;
	case TEILSPANNUNG5:		value = 5100;	break;
	case TEILSPANNUNG6:		value = 5100;	break;
	case TEILSPANNUNG7:		value = 5100;	break;
			
	case TEMPERATUR1:		value = 2000;	break;
	case TEMPERATUR2:		value = 2000;	break;
	case TEMPERATUR3:		value = 2000;	break;
	case TEMPERATUR4:		value = 2000;	break;
	case TEMPERATUR5:		value = 2000;	break;
	case TEMPERATUR6:		value = 2000;	break;
	case TEMPERATUR7:		value = 2000;	break;
	case TEMPERATUR8:		value = 2000;	break;
	case TEMPERATUR9:		value = 2000;	break;
	case TEMPERATUR10:		value = 2000;	break;
	case TEMPERATUR11:		value = 2000;	break;
	case TEMPERATUR12:		value = 2000;	break;
	case TEMPERATUR13:		value = 2000;	break;
	case TEMPERATUR14:		value = 2000;	break;
	
	case PC_CALIBR_TEMP:		value = 0x5678;			break;
	case MAX_BAT_TEMP:		value = 2000;			break;
	case UMGEBUNGS_TEMP:		value = getTemperature();	break;
	case MAX_LADETEMP:		value = 4500;			break;
	case MIN_LADETEMP:		value = 0;			break;
	case MAX_FAHRTEMP:		value = 4500;			break;
	case MIN_FAHRTEMP:		value = 0;			break;
	case MAX_LAGERTEMP:		value = 4500;			break;
	case MIN_LAGERTEMP:		value = 0;			break;

	case MAX_KAPAZITAET:		value = 305;	break;
	case MIN_KAPAZITAET:		value = 280;	break;
	case GELADENE_AH:		value = 1000;	break;
	case ENTLADENE_AH:		value = 800;	break;
	case LADEZYKLEN:		value = 100;	break;
	case TIEFENTLADE_ZYKLEN:	value = 0;	break;
	
	case MAX_ENTLADE_STROM:		value = -1000;	break;
				
	case ZYKLUS_UEBER_110:		value = 0;	break;
	case ZYKLUS_UEBER_100:		value = 10;	break;
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

	case MAX_UEBERLADUNG:		value = 5000;	break;
	case MIN_LDG_F_VOLL:		value = -500;	break;
	case STROM_ZUNAHME:		value = 40;	break;
	case MAX_LADE_SPG:		value = 44000;	break;
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
			
	case LADE_SPG_UNTER_M10:	value = 44000;	break;
	case LADE_SPG_UEBER_M10:	value = 44000;	break;
	case LADE_SPG_UEBER_P00:	value = 44000;	break;
	case LADE_SPG_UEBER_P10:	value = 44000;	break;
	case LADE_SPG_UEBER_P20:	value = 44000;	break;
	case LADE_SPG_UEBER_P30:	value = 42000;	break;
	case LADE_SPG_UEBER_P40:	value = 40000;	break;
	case LADE_SPG_UEBER_P45:	value = 38000;	break;
	case LADE_SPG_UEBER_P50:	value = 36000;	break;
			
	case NOM_KAPAZITAET:		value = 280;	break;
	case MIN_FAHR_SPANNUNG:		value = 31000;	break;
	case SELBST_ENTL_STROM:		value = 28000;	break;
	case TIEFENTLADE_SPG:		value = 25000;	break;
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
	}

	os_exitCS();

	return value;
}

void setBInfo(uint8_t bitNo)
{
	os_enterCS();
	battery.binfo |= (1<<bitNo);
	os_exitCS();
}

void clearBInfo(uint8_t bitNo)
{
	os_enterCS();
	battery.binfo &= ~(1<<bitNo);
	os_exitCS();
}
