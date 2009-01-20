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
 * @file 	mediator.c
 * @brief 	Main twike master application.
 * @author 	Markus Walser (markus.walser@gmail.com)
 * @date 	11.02.08
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include "mediator.h"
#include <avr/eeprom.h>
#include "i2c.h"
#include "adc.h"
#include "uart.h"
#include "timer.h"
#include "os_thread.h"
#include "command.h"
#include "sensors.h"
#include "simulator.h"
#include "delay.h"
#include "protocol.h"
#include "battery.h"

EPowerState ePowerIst;


int main(void)
{
	PORTA=0; DDRA=0;		//Alles Eingang alles Null
	PORTB=0; DDRB=0;
	PORTC=0; DDRC=0;
	PORTD=0; DDRD=0;

	// Disable JTag to allow normal usage of Port C.		
	MCUCR = 1<< JTD;
	MCUCR = 1<< JTD;

	wdt_disable();	// Watchdog aus!

	// adc_calibrate_offset();

	os_create_thread((uint8_t *)SP, NULL);	// main Hauptthread anlegen mit höchster Priorität.

	timer_2_init();
	
	/* Ist das ein Power on Reset? */
	#ifdef __AVR_ATmega644P__
		if ((MCUSR & 1) == 1) {
			MCUSR &= ~1;	// Bit loeschen
	#else
		if ((MCUCSR & 1) == 1) {
			MCUCSR &= ~1;	// Bit loeschen
	#endif
		delay(100);
		asm volatile("jmp 0");
	}

	delay(100);


	DDRB  |=  RELAIS; // Output
	DDRB  |=  IGBT; // Output
	DDRD  &= ~STOP_SWITCH; // Input
	PORTD |=  STOP_SWITCH; // Pullup enabled
	DDRD  |=  POWER_ONE_WIRE; // Output
	PORTD |=  POWER_ONE_WIRE; // Power on

	DDRC  |=  LED_RED;   // Output
	DDRC  |=  LED_GREEN; // Output
	PORTC &= ~LED_RED;   // Switch off red led.
	PORTC &= ~LED_GREEN; // Switch off green led.

	battery_init();	

	uart_init();
	

	adc_init(CH_TEMPERATURE); // Temperatur sensor
	adc_init(CH_VOLTAGE);     // Voltage
	adc_init(CH_CHARGE);      // Current +
	adc_init(CH_DISCHARGE);   // Current -

	cmd_init();

	i2c_init(100000); // 100kHz Speed

	sensors_init();

	ePowerIst = ePowerOff;
	battery_info_set(BAT_REL_OPEN);    // Atomic update of battery info

	EICRA = (1<<ISC01) | (1<<ISC10); // Configure any change on pin INT0 for interrupt.
	EIMSK = (1<<INT0);               // Enable INT0 interrupt

	ePowerSoll = ePowerSave;

	while(1)
	{
		if( ePowerSoll != ePowerIst)
		{
			if( ePowerSoll == ePowerFull )
			{
				PORTB |= IGBT;             // Switch IGBT on
				PORTB |= RELAIS;           // Switch Relais on
				os_thread_sleep(500);      // Relais delay time
				battery_info_clear(BAT_REL_OPEN);  // Atomic update of battery info
			}
			else if( ePowerSoll == ePowerSave )
			{
				battery_info_set(BAT_REL_OPEN);    // Atomic update of battery info
				PORTB |= IGBT;             // Switch IGBT on
				PORTB &= ~RELAIS;          // Switch Relais off				
			}
			else
			{
				battery_info_set(BAT_REL_OPEN);    // Atomic update of battery info
				PORTB &= ~RELAIS;          // Switch Relais off
				os_thread_sleep(500);      // Relais spark quenching
				PORTB &= ~IGBT;            // Switch IGBT off
				os_thread_sleep(5000);     // Let IGBT cool down, in case we're still powered.
			}
			ePowerIst = ePowerSoll;
		}
		os_thread_sleep(100);
		//os_thread_sleep(500);	// Hello world.
		//PORTC ^= LED_RED; 	// toggle red led.
		//uart_write("Hallo\n", 6);
	}
	return 0;
}


/*!
 * Interrupt-Handler fuer den Interrupt 0. Trennt den das ganze Twike von der Batterie.
 */
ISR (INT0_vect)
{
	ePowerSoll = ePowerOff;
}
