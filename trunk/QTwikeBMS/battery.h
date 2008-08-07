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
#ifndef BATTERY_H_
#define BATTERY_H_

#include "types.h"

#ifdef __cplusplus
 extern "C"
 {
#endif

typedef struct 
{
	uint16_t voltage;
	int16_t  current;
	int16_t  ah_counter;
	int16_t  temerature;
	uint8_t  address;
	uint8_t  drive_state;
	uint8_t  relais_state;
} battery_t;

extern battery_t battery1;
extern battery_t battery2;
extern battery_t battery3;

void setParameterValue(battery_t* pBattery, uint8_t parameter, uint16_t value);
int16_t getParameterValue(uint8_t parameter, battery_t* pBattery);

#ifdef __cplusplus
 }
#endif
 
#endif /*BATTERY_H_*/
