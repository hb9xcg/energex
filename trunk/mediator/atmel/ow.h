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

#ifndef OW_H_DEF
#define OW_H_DEF

#include "mediator.h"

/*! 
 * @file        ow.c
 * @brief       OneWire-Treiber
 * @author      Markus Walser (markus.walser@gmail.com)
 * @date        18.11.2008
 */

typedef enum 
{
	eOWSuccess         =  0,
	eOWNoResponse      = -1,
	eOWFailure         = -2,
	eOWNoPresencePulse = -3
} EOWError;

typedef enum
{
	eOWNormal = 0,
	eOWPowerDelivery = 1
} EOWLevel;

int8_t ow_restart(void);

EOWError ow_reset(void);

int8_t ow_write_bit(uint8_t bit);

int8_t ow_read_bit(void);

EOWError ow_write_byte(uint8_t write_byte);

EOWError ow_read_byte(uint8_t* read_byte);

void ow_block(void);

int8_t ow_search(uint8_t reset_search, uint8_t* last_device, uint8_t* device_address);

int8_t ow_level( uint8_t config_state, EOWLevel level);

void ow_read_bit_power(void);

EOWError ow_write_byte_power(uint8_t config_state, uint8_t write_byte);

#endif
