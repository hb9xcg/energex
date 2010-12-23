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
#ifndef BATTERY_H_
#define BATTERY_H_

#include "global.h"
#include "protocol.h"

#ifdef __cplusplus
 extern "C"
 {
#endif
	
#define TIEFENTLADE_SPANNUNG 	29000

typedef struct
{
	uint16_t rekuperation_nok :   1; // Rekuperation nicht erlaubt
	uint16_t charge_nok       :   1; // Ladung nicht erlaubt
	uint16_t drive_nok        :   1; // Fahren nicht erlaubt
	uint16_t charge_cur_to_hi :   1; // Zu hoher Ladestrom
	uint16_t drive_cur_to_hi  :   1; // Zu hoher Fahrstrom
	uint16_t voltage_to_hi    :   1; // Zu hohe Ladespannung
	uint16_t voltage_to_lo    :   1; // Zu tiefe Fahrspannung
	uint16_t bat_rel_open     :   1; // Batterie Relais offen
	uint16_t bat_full         :   1; // Batterie voll
	uint16_t bat_empty        :   1; // Batterie entladen
	uint16_t charge_temp_to_hi:   1; // Zu hohe Ladetemperatur
	uint16_t charge_temp_to_lo:   1; // Zu tiefe Ladetemperatur
	uint16_t drive_temp_to_hi :   1; // Zu hohe Fahrtemperatur
	uint16_t drive_temp_to_lo :   1; // Zu tiefe Fahrtemperatur
	uint16_t voltage_nok      :   1; // Unsymmetrische Spannungen
	uint16_t bat_error        :   1; // Fehler in der Batterieüberwachung
} T_BInfo;



typedef struct 
{
	uint16_t	voltage;
	int16_t		current;
	int16_t		ah_counter;
	int16_t		temerature;
	uint8_t		address;
	uint8_t		relais_state;
	uint16_t	binfo;
	int16_t         temperatur_1;
	int16_t         temperatur_2;
	int16_t         temperatur_3;
	int16_t         temperatur_4;
	int16_t         temperatur_5;
	int16_t         temperatur_6;
	int16_t         temperatur_7;
	int16_t         temperatur_8;
	int16_t         temperatur_9;
	int16_t         temperatur_10;
	int16_t         temperatur_11;
	int16_t         temperatur_12;
	int16_t         temperatur_13;
	int16_t         temperatur_14;
} battery_t;

extern battery_t battery;

void     battery_set_parameter_value(uint8_t parameter, uint16_t value);
int16_t  battery_get_parameter_value(uint8_t parameter);
void     battery_init(void);
void     battery_set_current(uint16_t current);
int16_t  battery_get_current(void);
void     battery_set_voltage(uint16_t voltage);
uint16_t battery_get_voltage(void);
void     battery_set_info(uint16_t binfo);
uint16_t battery_get_info(void);
void     battery_info_set(uint8_t bitNo);
void     battery_info_clear(uint8_t bitNo);
uint8_t  battery_info_get(uint8_t bitNo);

#ifdef __cplusplus
 }
#endif
 
#endif /*BATTERY_H_*/
