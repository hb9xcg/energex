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
#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include "global.h"

#ifdef __cplusplus
 extern "C"
 {
#endif

// Protocol
#define FRAME               0x10
#define LENGTH_MASK         0x0F

// Address
#define BATTERY_1           0x31
#define BATTERY_2           0x32
#define BATTERY_3           0x34
#define BROADCAST           0xFF

// Commands
#define ACK                 0x20 // Acknowledged
#define NAK                 0xF2 // Not Acknowledged
#define REQ_DATA            0x22 // Request Data
#define REQ_GROUP           0x42 // Request Group
#define TRM_DATA            0x24 // Reply Data
#define TRM_GROUP           0x48 // Reply Group

// Parameters:
#define MODEL_TYPE          0x01
#define PROGRAM_REV         0x02
#define PAR_TAB_REV         0x03
#define NENNSPNG            0x04
#define NENNSTROM           0x05
#define SERIE_NUMMER        0x06
#define REP_DATUM           0x07 
#define STANDZEIT           0x08
#define FAHR_LADE_ZEIT      0x09
#define LAST_ERROR          0x0a
#define BUS_ADRESSE         0x0b

#define DRIVE_STATE         0x11
#define COMMAND             0x12
#define PARAM_PROT          0x13

#define BINFO               0x21
#define IST_STROM           0x22	
#define LADESTROM           0x23
#define FAHRSTROM           0x24
#define TOTAL_SPANNUNG      0x25
#define SOLL_LADESPG        0x26
#define AH_ZAEHLER          0x27
#define Q                   0x28
#define LEISTUNG            0x29
#define BATTERIE_TEMP       0x2a
#define FINFO               0x2b

#define SYM_SPANNUNG        0x31		
#define TEILSPANNUNG1       0x32
#define TEILSPANNUNG2       0x33
#define TEILSPANNUNG3       0x34
#define TEILSPANNUNG4       0x35
#define TEILSPANNUNG5       0x36
#define TEILSPANNUNG6       0x37
#define TEILSPANNUNG7       0x38
	
#define TEMPERATUR1         0x41
#define TEMPERATUR2         0x42
#define TEMPERATUR3         0x43
#define TEMPERATUR4         0x44
#define TEMPERATUR5         0x45
#define TEMPERATUR6         0x46
#define TEMPERATUR7         0x47
#define TEMPERATUR8         0x51
#define TEMPERATUR9         0x52
#define TEMPERATUR10        0x53
#define TEMPERATUR11        0x54
#define TEMPERATUR12        0x55
#define TEMPERATUR13        0x56
#define TEMPERATUR14        0x57
		
#define PC_CALIBR_TEMP      0x60
#define MAX_BAT_TEMP        0x61
#define UMGEBUNGS_TEMP      0x62
#define MAX_LADETEMP        0x63
#define MIN_LADETEMP        0x64
#define MAX_FAHRTEMP        0x65
#define MIN_FAHRTEMP        0x66
#define MAX_LAGERTEMP       0x67
#define MIN_LAGERTEMP       0x68

#define MAX_KAPAZITAET      0x71
#define MIN_KAPAZITAET      0x72
#define GELADENE_AH         0x73
#define ENTLADENE_AH        0x74
#define LADEZYKLEN          0x75
#define TIEFENTLADE_ZYKLEN  0x76
#define MAX_ENTLADE_STROM   0x77
		
#define ZYKLUS_UEBER_110    0x81 	
#define ZYKLUS_UEBER_100    0x82
#define ZYKLUS_UEBER_90     0x83
#define ZYKLUS_UEBER_80     0x84
#define ZYKLUS_UEBER_70     0x85
#define ZYKLUS_UEBER_60     0x86
#define ZYKLUS_UEBER_50     0x87
#define ZYKLUS_UEBER_40     0x88
#define ZYKLUS_UEBER_30     0x89
#define ZYKLUS_UEBER_20     0x8a
#define ZYKLUS_UEBER_10     0x8b
#define ZYKLUS_UNTER_10     0x8c

#define MAX_UEBERLADUNG     0x91
#define MIN_LDG_F_VOLL      0x92
#define STROM_ZUNAHME       0x93
#define MAX_LADE_SPG        0x94
#define MIN_LADE_TEMP       0x95
#define MAX_LADE_TEMP       0x96
#define MAX_TEMP_ZUNAHME    0x97
#define MAX_LADEZEIT        0x98
#define SYMMETRIER_STROM    0x99

#define LADE_STR_UNTER_M10  0xa1
#define LADE_STR_UEBER_M10  0xa2
#define LADE_STR_UEBER_P00  0xa3
#define LADE_STR_UEBER_P10  0xa4
#define LADE_STR_UEBER_P20  0xa5
#define LADE_STR_UEBER_P30  0xa6
#define LADE_STR_UEBER_P40  0xa7
#define LADE_STR_UEBER_P45  0xa8
#define LADE_STR_UEBER_P50  0xa9
	
#define LADE_SPG_UNTER_M10  0xb1
#define LADE_SPG_UEBER_M10  0xb2
#define LADE_SPG_UEBER_P00  0xb3
#define LADE_SPG_UEBER_P10  0xb4
#define LADE_SPG_UEBER_P20  0xb5
#define LADE_SPG_UEBER_P30  0xb6
#define LADE_SPG_UEBER_P40  0xb7
#define LADE_SPG_UEBER_P45  0xb8
#define LADE_SPG_UEBER_P50  0xb9
	
#define NOM_KAPAZITAET      0xc1
#define MIN_FAHR_SPANNUNG   0xc2 
#define SELBST_ENTL_STROM   0xc3
#define TIEFENTLADE_SPG     0xc4
#define MAX_SPANNUNG_DIFF   0xc5
#define MIN_FAHR_TEMP_B     0xc6
#define MAX_FAHR_TEMP_B     0xc7
#define MAX_FAHR_STROM      0xc8

#define AD_STROM            0xe0

#define KAL_TEMP_7_6        0xd1
#define KAL_TEMP_5_4        0xd2
#define KAL_TEMP_3_2        0xd3
#define KAL_TEMP_1_AMB      0xd4
#define KAL_TEMP_GD_14      0xd5
#define KAL_TEMP_13_12      0xd6
#define KAL_TEMP_11_10      0xd7
#define KAL_TEMP_9_8        0xd8
#define SENSOR_MASK         0xd9
	
#define OFFS_KLEIN_STL      0xe1
#define OFFS_GROSS_STL      0xe2
#define KALIB_SPG_1         0xe3
#define KALIB_SPG_2         0xe4
#define KALIB_SPG_3         0xe5
#define KALIB_SPG_4         0xe6
#define KALIB_SPG_5         0xe7
#define KALIB_SPG_6         0xe8
#define KALIB_SPG_9         0xe9
	
#define DEBUG_VALUE_C       0xf1
#define DEBUG_VALUE_H       0xf2
#define DEBUG_VALUE_ADDR    0xf3

#define GESCHWINDIGKEIT     0xFE
#define TAGESKILOMETER      0xFF

// BatterieInfo / FahrInfo (BitNo)
#define	 REKUPERATION_NOK    0 // Rekuperation nicht erlaubt
#define	 CHARGE_NOK          1 // Ladung nicht erlaubt
#define	 DRIVE_NOK           2 // Fahren nicht erlaubt
#define	 CHARGE_CUR_TO_HI    3 // Zu hoher Ladestrom, reduziert Ladestrom
#define	 DRIVE_CUR_TO_HI     4 // Zu hoher Fahrstrom, reduziert Fahrstrom
#define	 VOLTAGE_TO_HI       5 // Zu hohe Ladespannung, aktiviert U-Ladung
#define	 VOLTAGE_TO_LO       6 // Zu tiefe Fahrspannung
#define	 BAT_REL_OPEN        7 // Batterie Relais offen
#define	 BAT_FULL            8 // Batterie voll
#define	 BAT_EMPTY           9 // Batterie entladen
#define	 CHARGE_TEMP_TO_HI  10 // Zu hohe Ladetemperatur
#define	 CHARGE_TEMP_TO_LO  11 // Zu tiefe Ladetemperatur
#define	 DRIVE_TEMP_TO_HI   12 // Zu hohe Fahrtemperatur
#define	 DRIVE_TEMP_TO_LO   13 // Zu tiefe Fahrtemperatur
#define	 VOLTAGE_NOK        14 // Unsymmetrische Spannungen
#define	 BAT_ERROR          15 // Fehler in der Batterieüberwachung


typedef enum
{
	eConverterOff   =  0, // InvOff           Umrichter aus
	eConverterTest  =  1, // InvTest          Test-Modus
	eConverterProg  =  2, // InvProg          Programm-Modus
	eConverterIdle  =  3, // InvIdle          Umschaltungszustand vor dem Fahren
	eBreakDown      =  4, // BreakDown        Fehler des Umrichters
	eDrive          =  5, // Drive            Fahren
	eReadyCharge    =  6, // ReadyCharge      warten auf Netzspannung
	ePreCharge      =  7, // PreCharge        Vorladung
	eClosePCRelais  =  8, // CloseVRelais     Vorladerelais schliessen
	eICharge        =  9, // ICharge          I-Ladung
	eUCharge        = 10, // UCharge          U-Ladung
	eOpenBatRelais  = 11, // OpenBRelais      Batterierelais öffnen
	ePostCharge     = 12, // PostCharge       Nachladung
	eCloseBatRelais = 13, // CloseBRelais     Batterierelais schliessen
	eSymCharge      = 14, // SymmCharge       Symmetrierladung
	eTrickleCharge  = 15, // TrickleCharge    Erhaltungsladung
	eTrickleWait    = 16, // TrickleWait      Pause
	eUnknown17      = 17, // (?)              (möglicherweise Erhaltungsladung einzelner Blöcke?)
	eUnknown18      = 18, // (?)              (möglicherweise Erhaltungsladung einzelner Blöcke?)
	eUnknown19      = 19, // (?)              (möglicherweise Erhaltungsladung einzelner Blöcke?)
	eUnknown20      = 20  // (?)
} EDriveState;


extern void protocol_receive_byte(uint8_t character);

#ifdef __cplusplus
 }
#endif
 
#endif /*PROTOCOL_H_*/
