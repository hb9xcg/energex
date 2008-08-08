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

battery_t battery;

void setParameterValue(uint8_t parameter, uint16_t value)
{
	switch(parameter)
	{
	case DRIVE_STATE:	battery.drive_state  	= value;	break;
	case RELAIS_STATE:	battery.relais_state 	= value;	break;
	case BUS_ADRESSE:	battery.address 	= value;	break;
	case TOTAL_SPANNUNG:	battery.voltage 	= value;	break;
	default:
		break;
	}	
}

int16_t getParameterValue(uint8_t parameter)
{
	switch(parameter)
	{						
	case MODEL_TYPE:		return 1;
	case PROGRAM_REV:		return 510;
	case PAR_TAB_REV:		return 510;
	
	case NENNSPNG:			return 33600;		
	case NENNSTROM:			return 280;
	case SERIE_NUMMER:		return 40847;
	case REP_DATUM: 		return 2;
	case STANDZEIT:			return 64;
	case FAHR_LADE_ZEIT:		return 9;
	case LAST_ERROR:		return 0;
	case BUS_ADRESSE:		return battery.address;

	case DRIVE_STATE:		return battery.drive_state;
	case RELAIS_STATE:		return battery.relais_state;
	case PARAM_PROT:		return 0;

	case BINFO:			return 0;
	
	// We simulate 3 batteries. Thus each of them reports one third of the real value.
	case IST_STROM:			return battery.current/3; 			
	case LADESTROM:			return 280;
	case FAHRSTROM:			return -1000;
	case TOTAL_SPANNUNG:		return battery.voltage;
	case SOLL_LADESPG:		return	44000;
	
	 // We simulate 3 batteries. Thus each of them reports one third of the real value.
	case AH_ZAEHLER:		return battery.ah_counter/3;
	case Q:				return -1400;
	case LEISTUNG:			return (battery.current/100)*(battery.voltage/100);
	case BATTERIE_TEMP:		return 2000;
	case FINFO:			return 0;
	case SYM_SPANNUNG:		return 26;
				
	case TEILSPANNUNG1:		return 5100;
	case TEILSPANNUNG2:		return 5100;
	case TEILSPANNUNG3:		return 5100;
	case TEILSPANNUNG4:		return 5100;
	case TEILSPANNUNG5:		return 5100;
	case TEILSPANNUNG6:		return 5100;
	case TEILSPANNUNG7:		return 5100;
			
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
	
	case PC_CALIBR_TEMP:		return 0x5678;
	case MAX_BAT_TEMP:		return 2000;
	case UMGEBUNGS_TEMP:		return 2000;	
	case MAX_LADETEMP:		return 4500;
	case MIN_LADETEMP:		return 0;
	case MAX_FAHRTEMP:		return 4500;
	case MIN_FAHRTEMP:		return 0;
	case MAX_LAGERTEMP:		return 4500;
	case MIN_LAGERTEMP:		return 0;

	case MAX_KAPAZITAET:		return 305;
	case MIN_KAPAZITAET:		return 280;
	case GELADENE_AH:		return 1000;
	case ENTLADENE_AH:		return 800;
	case LADEZYKLEN:		return 100;
	case TIEFENTLADE_ZYKLEN:	return 0;
	
	case MAX_ENTLADE_STROM:		return -1000;
				
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

	case MAX_UEBERLADUNG:		return 5000;
	case MIN_LDG_F_VOLL:		return -500;
	case STROM_ZUNAHME:		return 40;
	case MAX_LADE_SPG:		return 44000;
	case MIN_LADE_TEMP:		return 0;
	case MAX_LADE_TEMP:		return 4500;
	case MAX_TEMP_ZUNAHME:		return 100;
	case MAX_LADEZEIT:		return 800;
	case SYMMETRIER_STROM:		return 28;

	case LADE_STR_UNTER_M10:	return 56;
	case LADE_STR_UEBER_M10:	return 56;
	case LADE_STR_UEBER_P00:	return 280;
	case LADE_STR_UEBER_P10:	return 280;
	case LADE_STR_UEBER_P20:	return 280;
	case LADE_STR_UEBER_P30:	return 280;
	case LADE_STR_UEBER_P40:	return 79;
	case LADE_STR_UEBER_P45:	return 28;
	case LADE_STR_UEBER_P50:	return 28;
			
	case LADE_SPG_UNTER_M10:	return 44000;
	case LADE_SPG_UEBER_M10:	return 44000;
	case LADE_SPG_UEBER_P00:	return 44000;
	case LADE_SPG_UEBER_P10:	return 44000;
	case LADE_SPG_UEBER_P20:	return 44000;
	case LADE_SPG_UEBER_P30:	return 42000;
	case LADE_SPG_UEBER_P40:	return 40000;
	case LADE_SPG_UEBER_P45:	return 38000;
	case LADE_SPG_UEBER_P50:	return 36000;
			
	case NOM_KAPAZITAET:		return 280;
	case MIN_FAHR_SPANNUNG:		return 31000;
	case SELBST_ENTL_STROM:		return 28000;
	case TIEFENTLADE_SPG:		return 25000;
	case MAX_SPANNUNG_DIFF:		return 500;
	case MIN_FAHR_TEMP_B:		return -2500;
	case MAX_FAHR_TEMP_B:		return 6000;
	case MAX_FAHR_STROM:		return -1000;
	
	case AD_STROM:			return 0x817F;
	
	case KAL_TEMP_7_6:		return 0xFFFF;
	case KAL_TEMP_5_4:		return 0xFEFD;
	case KAL_TEMP_3_2:		return 0xFCFD;
	case KAL_TEMP_1_AMB:		return 0x00FE;
	case KAL_TEMP_GD_14:		return 0x7C00;
	case KAL_TEMP_13_12:		return 0x00FF;
	case KAL_TEMP_11_10:		return 0xFE00;
	case KAL_TEMP_9_8:		return 0x0002;

	case SENSOR_MASK:		return 0x0000;
			
	case OFFS_KLEIN_STL:		return 1508;
	case OFFS_GROSS_STL:		return 12523;
	case KALIB_SPG_1:		return 0x00A3;
	case KALIB_SPG_2:		return 0x009F;
	case KALIB_SPG_3:		return 0x00A6;
	case KALIB_SPG_4:		return 0x00B1;
	case KALIB_SPG_5:		return 0x007C;
	case KALIB_SPG_6:		return 0x00A7;
	case KALIB_SPG_9:		return 0x00A7;
			
	case DEBUG_VALUE_C:		return 22498;
	case DEBUG_VALUE_H:		return 0x57E2;
	case DEBUG_VALUE_ADDR:		return 0xFFFF;

	case GESCHWINDIGKEIT:		return 0;
	case TAGESKILOMETER:		return 0;
	
	default:
		return 0x0000;
	}
}
