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

/*! 
 * @file        sensors.h
 * @brief       OneWire DS18S20 Temperatursensor-Treiber
 * @author      Markus Walser (markus.walser@gmail.com)
 * @date        23.11.2008
 */

#ifndef SENSORS_H_DEF
#define SENSORS_H_DEF

#include "mediator.h"

typedef enum {
	eDSSuccess         =  0,
	eDSNoResponse      = -1,
	eDSFailure         = -2,
	eDSNoPresencePulse = -3,
	eDSCrcError        = -4
} EDSError;

void     sensors_init(void);
EDSError sensors_start_conversion(uint8_t serial[]);
EDSError sensors_fetch_conversion(uint8_t serial[], int16_t* temp);
void     sensors_wait_conversion(void);
int8_t   sensors_get_nbr_of_devices(void);
void     sensors_get_temperatur(int8_t index, int16_t* temp);
void     sensors_get_min_temperatur(int16_t* temp);
void     sensors_get_max_temperatur(int16_t* temp);
void     sensors_get_avg_temperatur(int16_t* temp);
void     sensors_get_serial(int8_t index, uint8_t serial[]);
uint16_t sensors_get_max_stack_usage(void);

#endif
