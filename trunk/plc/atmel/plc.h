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
 * @file 	plc.h
 * @brief 	Powerline controller.
 * @author 	Markus Walser (markus.walser@gmail.com)
 * @date 	20.10.2010
 */

#ifndef MEDIATOR_H_DEF
#define MEDIATOR_H_DEF

#define NEW_AVR_LIB
#define OS_AVAILABLE

#include "global.h"
#include "battery.h"
#include "io.h"

#include <avr/io.h>


#define ADC_AVAILABLE		/*!< A/D-Converter */

#define TWI_AVAILABLE

#ifdef TWI_AVAILABLE
	#define I2C_AVAILABLE	/*!< I2C-Treiber statt TWI-Implementierung benutzen */
#endif

#define XTAL F_CPU			 /*!< Crystal frequency in Hz */


void plc_set_drive_state(EDriveState eState);
EDriveState plc_get_drive_state(void);
extern int16 plc_temperature;
extern int8_t plc_enable_sampling;
void plc_busy(void);
void plc_force_busy(int8_t on);
void plc_sample(void);
uint16_t plc_get_line_voltage(void);

#endif
