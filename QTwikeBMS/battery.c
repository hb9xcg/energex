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
#include "protocol.h"
#include <assert.h>
#include <stdio.h>

battery_t battery1;
battery_t battery2;
battery_t battery3;

void setParameterValue(battery_t* pBattery, uint8_t parameter, uint16_t value)
{
	switch(parameter)
	{
	case DRIVE_STATE:		pBattery->drive_state  	= value;	break;
	case RELAIS_STATE:		pBattery->relais_state 	= value;	break;
	case BUS_ADRESSE:		pBattery->address 		= value;	break;
	case TOTAL_SPANNUNG:	pBattery->voltage 		= value;	break;
	default:
		fprintf(stderr, "Writing unknown parameter %#x\n", parameter);
	}	
}

int16_t getParameterValue(uint8_t parameter, battery_t* pBattery)
{
	switch(parameter)
	{						
	case MODEL_TYPE:		return 1;
	case PROGRAM_REV:		return 510;
	case PAR_TAB_REV:		return 510;
	
	case NENNSPNG:			return 33600;		
	case NENNSTROM:		return 280;
	case SERIE_NUMMER:		return 40847;
	case REP_DATUM: 		return 2;
	case STANDZEIT:		return 64;
	case FAHR_LADE_ZEIT:	return 9;
	case LAST_ERROR:		return 0;
	case BUS_ADRESSE:		return pBattery->address;

	case DRIVE_STATE:		return pBattery->drive_state;
	case RELAIS_STATE:		return pBattery->relais_state;
	case PARAM_PROT:		return 0;

	case BINFO:			return 0;
	case IST_STROM:		return pBattery->current;			
	case LADESTROM:		return 280;
	case FAHRSTROM:		return -1000;
	case TOTAL_SPANNUNG:	return pBattery->voltage;
	case SOLL_LADESPG:		return	420;
	case AH_ZAEHLER:		return pBattery->ah_counter;
	case Q:				return 1000;
	case LEISTUNG:			return pBattery->current*(pBattery->voltage/100);
	case BATTERIE_TEMP:	return 20;
	case FINFO:			return 0;
	case SYM_SPANNUNG:		return 0;
				
	case TEILSPANNUNG1:	return 5100;
	case TEILSPANNUNG2:	return 5100;
	case TEILSPANNUNG3:	return 5100;
	case TEILSPANNUNG4:	return 5100;
	case TEILSPANNUNG5:	return 5100;
	case TEILSPANNUNG6:	return 5100;
	case TEILSPANNUNG7:	return 5100;
			
	case TEMPERATUR1:		return 2000;
	case TEMPERATUR2:		return 2000;
	case TEMPERATUR3:		return 2000;
	case TEMPERATUR4:		return 2000;
	case TEMPERATUR5:		return 2000;
	case TEMPERATUR6:		return 2000;
	case TEMPERATUR7:		return 2000;
	case TEMPERATUR8:		return 2000;
	case TEMPERATUR9:		return 2000;
	case TEMPERATUR10:		return 2000;
	case TEMPERATUR11:		return 2000;
	case TEMPERATUR12:		return 2000;
	case TEMPERATUR13:		return 2000;
	case TEMPERATUR14:		return 2000;
	
	case PC_CALIBR_TEMP:	return 0x5678;
	case MAX_BAT_TEMP:		return 2000;
	case UMGEBUNGS_TEMP:	return 2000;	
	case MAX_LADETEMP:		return 4500;
	case MIN_LADETEMP:		return 0;
	case MAX_FAHRTEMP:		return 4500;
	case MIN_FAHRTEMP:		return 0;
	case MAX_LAGERTEMP:	return 4500;
	case MIN_LAGERTEMP:	return 0;

	case MAX_KAPAZITAET:	return 305;
	case MIN_KAPAZITAET:	return 280;
	case GELADENE_AH:		return 1000;
	case ENTLADENE_AH:		return 800;
	case LADEZYKLEN:		return 100;
	case TIEFENTLADE_ZYKLEN:	return 0;
	
	case MAX_ENTLADE_STROM:	return -1000;
				
	case ZYKLUS_UEBER_110:		return 0;
	case ZYKLUS_UEBER_100:		return 10;
	case ZYKLUS_UEBER_90:		return 0;
	case ZYKLUS_UEBER_80:		return 0;
	case ZYKLUS_UEBER_70:		return 0;
	case ZYKLUS_UEBER_60:		return 0;
	case ZYKLUS_UEBER_50:		return 0;
	case ZYKLUS_UEBER_40:		return 0;
	case ZYKLUS_UEBER_30:		return 0;
	case ZYKLUS_UEBER_20:		return 0;
	case ZYKLUS_UEBER_10:		return 0;
	case ZYKLUS_UNTER_10:		return 0;

	case MAX_UEBERLADUNG:		return 1;
	case MIN_LDG_F_VOLL:		return 2;
	case STROM_ZUNAHME:		return 3;
	case MAX_LADE_SPG:			return 4;
	case MIN_LADE_TEMP:		return 5;
	case MAX_LADE_TEMP:		return 6;
	case MAX_TEMP_ZUNAHME:		return 7;
	case MAX_LADEZEIT:			return 8;
	case SYMMETRIER_STROM:		return 70;

	case LADE_STR_UNTER_M10:	return 70;
	case LADE_STR_UEBER_M10:	return 70;
	case LADE_STR_UEBER_P00:	return 500;
	case LADE_STR_UEBER_P10:	return 500;
	case LADE_STR_UEBER_P20:	return 500;
	case LADE_STR_UEBER_P30:	return 500;
	case LADE_STR_UEBER_P40:	return 500;
	case LADE_STR_UEBER_P45:	return 70;
	case LADE_STR_UEBER_P50:	return 0;
			
	case LADE_SPG_UNTER_M10:	return 420;
	case LADE_SPG_UEBER_M10:	return 420;
	case LADE_SPG_UEBER_P00:	return 420;
	case LADE_SPG_UEBER_P10:	return 420;
	case LADE_SPG_UEBER_P20:	return 420;
	case LADE_SPG_UEBER_P30:	return 420;
	case LADE_SPG_UEBER_P40:	return 420;
	case LADE_SPG_UEBER_P45:	return 420;
	case LADE_SPG_UEBER_P50:	return 420;
			
	case NOM_KAPAZITAET:		return 5;
	case MIN_FAHR_SPANNUNG:	return 290;
	case SELBST_ENTL_STROM:	return 0;
	case TIEFENTLADE_SPG:		return 270;
	case MAX_SPANNUNG_DIFF:	return 5;
	case MIN_FAHR_TEMP_B:		return -20;
	case MAX_FAHR_TEMP_B:		return 50;
	case MAX_FAHR_STROM:		return 20;
	
	case AD_STROM:				return 0x1234;
	
	case KAL_TEMP_7_6:			return 0;
	case KAL_TEMP_5_4:			return 0;
	case KAL_TEMP_3_2:			return 0;
	case KAL_TEMP_1_AMB:		return 0;
	case KAL_TEMP_GD_14:		return 0;
	case KAL_TEMP_13_12:		return 0;
	case KAL_TEMP_11_10:		return 0;
	case KAL_TEMP_9_8:			return 0;

	case SENSOR_MASK:			return 0;
			
	case OFFS_KLEIN_STL:		return 0;
	case OFFS_GROSS_STL:		return 0;
	case KALIB_SPG_1:			return 0;
	case KALIB_SPG_2:			return 0;
	case KALIB_SPG_3:			return 0;
	case KALIB_SPG_4:			return 0;
	case KALIB_SPG_5:			return 0;
	case KALIB_SPG_6:			return 0;
	case KALIB_SPG_9:			return 0;
			
	case DEBUG_VALUE_C:		return 0;
	case DEBUG_VALUE_H:		return 0;
	case DEBUG_VALUE_ADDR:		return 0;

	case GESCHWINDIGKEIT:		return 0;
	case TAGESKILOMETER:		return 0;
	
	default:
		printf("Retrieving unknown parameter %#x\n", parameter);
		return 0xff;
	}
}
