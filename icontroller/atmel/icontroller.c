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
 * @file 	icontroller.c
 * @brief 	Main twike master application.
 * @author 	Markus Walser (markus.walser@gmail.com)
 * @date 	11.02.08
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include "icontroller.h"
#include <avr/eeprom.h>
#include "i2c.h"
#include "adc.h"
#include "uart.h"
#include "timer.h"
#include "os_thread.h"
#include "command.h"
#include "supervisor.h"
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

	wdt_disable();	// Watchdog aus!

	os_create_thread((uint8_t *)SP, NULL);	// main Hauptthread anlegen mit höchster Priorität.

	timer_2_init();
	
	/* Ist das ein Power on Reset? */
	#ifdef __AVR_ATmega644__
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

	DDRD  |=  RELAIS; // Output
	DDRD  |=  IGBT; // Output
	DDRD  &= ~STOP_SWITCH; // Input
	PORTD |=  STOP_SWITCH; // Pullup enabled

	uart_init();

	adc_init(CH_TEMPERATURE); // Temperatur sensor
	adc_init(CH_VOLTAGE);     // Voltage
	adc_init(CH_CHARGE);      // Current +
	adc_init(CH_DISCHARGE);   // Current -

	cmd_init();

	i2c_init(100000); // 100kHz Speed

	supervisor_init();

	ePowerIst = ePowerOff;
	setBInfo(BAT_REL_OPEN);    // Atomic update of battery info

	EICRA = (1<<ISC01) | (1<<ISC10); // Configure any change on pin INT0 for interrupt.
	EIMSK = (1<<INT0);               // Enable INT0 interrupt

	os_thread_sleep(3000);
	ePowerSoll = ePowerSave;

	while(1)
	{
		if( ePowerSoll != ePowerIst)
		{
			if( ePowerSoll == ePowerFull )
			{
				PORTD |= IGBT;             // Switch IGBT on
				os_thread_sleep(10);       // Relais spark quenching
				PORTD |= RELAIS;           // Switch Relais on
				os_thread_sleep(300);      // Relais delay time
				clearBInfo(BAT_REL_OPEN);  // Atomic update of battery info
			}
			else if( ePowerSoll == ePowerSave )
			{
				setBInfo(BAT_REL_OPEN);    // Atomic update of battery info
				PORTD |= IGBT;             // Switch IGBT on
				PORTD &= ~RELAIS;          // Switch Relais off				
			}
			else
			{
				setBInfo(BAT_REL_OPEN);    // Atomic update of battery info
				PORTD &= ~RELAIS;          // Switch Relais off
				os_thread_sleep(300);      // Relais spark quenching
				PORTD &= ~IGBT;            // Switch IGBT off
				os_thread_sleep(5000);     // Let IGBT cool down, in case we're still powered.
			}
			ePowerIst = ePowerSoll;
		}
		os_thread_yield();
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
