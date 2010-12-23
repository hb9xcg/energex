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
 * @file 	io.c
 * @brief 	board specific io port configuration.
 * @author 	Markus Walser (markus.walser@gmail.com)
 * @date 	20.10.2010
 */
#include <avr/io.h>
#include "io.h"

void io_init(void)
{
	DDRA |= (uint8_t)(1<<PA7);  // Relais output
	DDRA |= (uint8_t)(1<<PA1);  // Green LED output
	DDRA |= (uint8_t)(1<<PA0);  // Red LED output

	io_clear_green_led();
	io_clear_red_led();
	io_open_relais();
}

void io_close_relais(void)
{
	PORTA |= (uint8_t)(1<<PA7);
}

void io_open_relais(void)
{
	PORTA &= (uint8_t)~(1<<PA7);
}

uint8_t io_is_relais_closed(void)
{
	return (PORTA & (uint8_t)(1<<PA7));
}

void io_set_green_led(void)
{
	PORTA |= (uint8_t)(1<<PA0);
}

void io_clear_green_led(void)
{
	PORTA &= (uint8_t)~(1<<PA0);
}

void io_toggle_green_led(void)
{
	PORTA ^= (uint8_t)(1<<PA0);
}

void io_set_red_led(void)
{
	PORTA |= (uint8_t)(1<<PA1);
}

void io_clear_red_led(void)
{
	PORTA &= (uint8_t)~(1<<PA1);
}

void io_toggle_red_led(void)
{
	PORTA ^= (uint8_t)(1<<PA1);
}

