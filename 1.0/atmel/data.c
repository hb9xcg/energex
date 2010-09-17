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
 * @file 	data.h
 * @brief 	Stores and loads persistent data.
 * @author 	Markus Walser (markus.walser@gmail.com)
 * @date 	22.02.08
 */

#include <avr/eeprom.h>
#include "data.h"
#include "charge.h"

// Declaration
//uint8_t eeI2CAddress EEMEM = DEFAULT_I2C_ADDRESS;
uint16_t eeCounter_10mAh   EEMEM = 0; // current battery charging level
uint16_t eeDischarged_Ah   EEMEM = 0; // current battery charging level
uint16_t eeCharged_Ah      EEMEM = 0; // current battery charging level


void data_save(void)
{
	eeprom_busy_wait();
	eeprom_write_word( &eeCounter_10mAh, charge_get_capacity() );
	eeprom_busy_wait();
	eeprom_write_word( &eeDischarged_Ah, charge_get_total_discharge() );
	eeprom_busy_wait();
	eeprom_write_word( &eeCharged_Ah, 0 );
	eeprom_busy_wait();
}

void data_load(void)
{
	uint16_t value;

	eeprom_busy_wait();
	value = eeprom_read_word( &eeCounter_10mAh);
	charge_set_capacity(value);

	eeprom_busy_wait();
	value = eeprom_read_word( &eeDischarged_Ah );
	charge_set_total_discharge(value);

	eeprom_busy_wait();
	value = eeprom_read_word( &eeCharged_Ah );
}

#if 0


// read
eeprom_busy_wait();
callibration = eeprom_read_word( &eeCalibration );
eeprom_busy_wait();
address = eeprom_read_byte( &eeI2CAddress );





#endif
