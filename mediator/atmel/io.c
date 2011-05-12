/*
 * Energex
 * 
 * Copyright (C) 2008-2011 by Markus Walser
 * markus.walser@gmail.com
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
 * @date 	16.11.2010
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include "io.h"
#include "data.h"

#define RELAIS		PB1
#define IGBT		PB2
#define LED_RED 	PD7
#define LED_GREEN	PD6
#define IO_POWER	PD4
#define STOP_SWITCH	PD2
#define EMERGENCY       PB0

static uint8_t io_stop;


void io_init(void)
{
        PORTA=0; DDRA=0;                        // Alles Eingang alles Null
        PORTB=0; DDRB=0;
        PORTC=0; DDRC=0;
        PORTD=0; DDRD=0;


        // Disable JTag to allow normal usage of Port C.
	MCUCR = 1<< JTD;
	MCUCR = 1<< JTD;


	DDRB |= (uint8_t)(1<<RELAIS);           // Relais output
	DDRB |= (uint8_t)(1<<IGBT);             // IGBT output
	
	DDRB |= (uint8_t)(1<<EMERGENCY);        // Emergency Output
	
	DDRD |= (uint8_t)(1<<LED_GREEN);        // Green LED output
	DDRD |= (uint8_t)(1<<LED_RED);          // Red LED output

	DDRD |= (uint8_t)(1<<IO_POWER);         // Interface power output

	DDRD  &= (uint8_t)~(1<<STOP_SWITCH);    // Input
	PORTD |= (uint8_t) (1<<STOP_SWITCH);    // Pullup enabled


	io_clear_green_led();
	io_clear_red_led();
	io_open_relais();
	io_disable_igbt();
	io_disable_interface_power();

	io_stop = io_get_stop();
}

void io_raise_emergency(void)
{
        PORTB &= (uint8_t)~(1<<EMERGENCY);  // Remove Twike control power
}

void io_release_emergency(void)
{
        PORTB |= (uint8_t)(1<<EMERGENCY);  // Remove Twike control power
}

void io_enable_interface_power(void)
{
	PORTD |= (uint8_t)(1<<IO_POWER);
}

void io_disable_interface_power(void)
{
	PORTD &= (uint8_t)~(1<<IO_POWER);
}

void io_enable_igbt(void)
{
// 	ATMega port is defect.
//	PORTB |= (uint8_t)(1<<IGBT);
}

void io_disable_igbt(void)
{
	PORTB &= (uint8_t)~(1<<IGBT);
}

void io_close_relais(void)
{
	PORTB |= (uint8_t)(1<<RELAIS);
}

void io_open_relais(void)
{
	PORTB &= (uint8_t)~(1<<RELAIS);
}

uint8_t io_is_relais_closed(void)
{
	return (PORTB & (uint8_t)(1<<RELAIS));
}

void io_set_green_led(void)
{
	PORTD |= (uint8_t)(1<<LED_GREEN);
}

void io_clear_green_led(void)
{
	PORTD &= (uint8_t)~(1<<LED_GREEN);
}

void io_toggle_green_led(void)
{
	PORTD ^= (uint8_t)(1<<LED_GREEN);
}

void io_set_red_led(void)
{
	PORTD |= (uint8_t)(1<<LED_RED);
}

void io_clear_red_led(void)
{
	PORTD &= (uint8_t)~(1<<LED_RED);
}

void io_toggle_red_led(void)
{
	PORTD ^= (uint8_t)(1<<LED_RED);
}

uint8_t io_get_stop(void)
{
	return (PIND & (uint8_t)(1<<STOP_SWITCH));
}

void io_enable_stop_interrupt(void)
{
        EICRA = (1<<ISC01) | (1<<ISC10); // Configure any change on pin INT0 for interrupt.
        EIMSK = (1<<INT0);               // Enable INT0 interrupt
}

/*!
 *  * Interrupt-Handler fuer den Interrupt 0. Trennt den das ganze Twike von der Batterie.
 *   */
ISR (INT0_vect)
{

	if (mediator_get_drive_state() == eConverterOff)
	{
		io_release_emergency();
	}
	else
	{
		io_raise_emergency();

		io_set_red_led();
		data_save();
		io_clear_red_led();

		ePowerSoll = ePowerSave;
		io_disable_interface_power();
		
		mediator_set_drive_state(eConverterOff);
	}
}

