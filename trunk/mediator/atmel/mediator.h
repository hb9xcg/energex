/*
 * Energex
 * 
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your
 * option) any later version. 
 * This program is distributed in the hope that it will be 
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
 * PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public 
 * License along with this program; if not, write to the Free 
 * Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307, USA.
 * 
 */

/*! 
 * @file 	icontroller.h
 * @brief 	Main twike master application.
 * @author 	Markus Walser (markus.walser@gmail.com)
 * @date 	11.02.08
 */

#ifndef I_CONTROLLER_H_DEF
#define I_CONTROLLER_H_DEF

#define MCU
#define NEW_AVR_LIB
#define OS_AVAILABLE

#include "global.h"

#define ADC_AVAILABLE		/*!< A/D-Converter */

#define TWI_AVAILABLE

#ifdef TWI_AVAILABLE
	#define I2C_AVAILABLE	/*!< I2C-Treiber statt TWI-Implementierung benutzen */
#endif

#define XTAL F_CPU			 /*!< Crystal frequency in Hz */

#define IGBT   		0x20
#define RELAIS 		0x40
#define STOP_SWITCH	0x04

typedef enum
{
	ePowerOff,
	ePowerSave,
	ePowerFull,
	ePowerLast
} EPowerState;

EPowerState ePowerSoll;

#endif
