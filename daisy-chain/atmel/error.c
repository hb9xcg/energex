
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
 * @file 	error.c
 * @brief 	Error handling.
 * @author 	Markus Walser (markus.walser@gmail.com)
 * @date 	19.09.2010
 */
#include <avr/io.h>
#include "battery.h"
#include "io.h"
#include "error.h"
#include "delay.h"


void fatal(uint8_t code)
{
	// Activate emergency stop
	io_raise_emergency();

	error(code);
}

void error(uint8_t code)
{
	for (;;)
	{
		io_set_red_led();
		delay(200);
		io_clear_red_led();

		for (uint8_t i=0; i<code; i++)
		{
			io_set_green_led();
			delay(200);
			io_clear_green_led();
			delay(200);
		}
		delay(200);
	}
}

void warning(void)
{
	battery_info_set(BAT_ERROR);
}

