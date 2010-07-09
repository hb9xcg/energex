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
 * @file 	timer.c
 * @brief 	Contains timer handler.
 * @author 	Markus Walser (markus.walser@gmail.com)
 * @date 	04.03.2008
 */

#include <avr/interrupt.h>

#include "led.h"

uint8_t load, watchdog;

void timer_initialize(void)
{
	// Setup 1/8s periodic timer
	TCCR1B |= (1<<CTC1)|(1<<CS13)|(1<<CS12)|(1<<CS10);
	OCR1C = 244;
	OCR1A = 0;
	TIMSK |= (1<<TOV1);
}


ISR(TIMER1_OVF1_vect)
{
	static uint8_t counter;

	if( counter >= load)
	{
		RED(OFF);
	}
	else
	{
		RED(ON);
	}

	counter++;
//	if(counter >= 9) // load 9 => 100%
	if(counter > 9) // load 10 => 100%
	{
		GREEN(ON);
		counter = 0;
		GREEN(OFF);
	}

	if( ++watchdog > 240 ) // At 8MHz: Switch off after 30s silence.
	{
		POWER(OFF);
	}
}
