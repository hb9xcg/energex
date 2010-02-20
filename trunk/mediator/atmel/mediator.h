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

/*! 
 * @file 	mediator.h
 * @brief 	Main twike master application.
 * @author 	Markus Walser (markus.walser@gmail.com)
 * @date 	11.02.08
 */

#ifndef MEDIATOR_H_DEF
#define MEDIATOR_H_DEF

#define MCU
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

int16_t mediator_get_temperature(void);

typedef enum
{
	ePowerOff,
	ePowerSave,
	ePowerFull,
	ePowerLast
} EPowerState;

EPowerState ePowerSoll;

void mediator_set_drive_state(EDriveState eState);
EDriveState mediator_get_drive_state(void);
void mediator_check_binfo(void);
extern int16 mediator_temperature;
void mediator_busy(void);
void mediator_force_busy(int8_t on);

#endif
