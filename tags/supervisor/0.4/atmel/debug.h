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
 * @file 	debug.h
 * @brief 	Contains some debug helper functions.
 * @author 	Markus Walser (markus.walser@gmail.com)
 * @date 	04.03.2008
 */
#define F_CPU 8000000UL  // 8 MHz
#include <util/delay.h>
#include "led.h"


void debug(char byte)
{
	char c;
	for(c=0; c<8; c++)
	{
		GREEN(ON);
		RED(byte & 1<<c);
		_delay_ms(260.0);_delay_ms(260.0);
		_delay_ms(260.0);_delay_ms(260.0);
		_delay_ms(260.0);_delay_ms(260.0);
		_delay_ms(260.0);_delay_ms(260.0);
		GREEN(OFF);
		_delay_ms(260.0);_delay_ms(260.0);
		_delay_ms(260.0);_delay_ms(260.0);
		_delay_ms(260.0);_delay_ms(260.0);
		_delay_ms(260.0);_delay_ms(260.0);
	}
}
