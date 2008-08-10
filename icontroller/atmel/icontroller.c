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

EPowerState ePowerIst;

int main(void)
{
	PORTA=0; DDRA=0;		//Alles Eingang alles Null
	PORTB=0; DDRB=0;
	PORTC=0; DDRC=0;
	PORTD=0; DDRD=0;

	wdt_disable();	// Watchdog aus!

	os_create_thread((uint8_t *)SP, NULL);	// Hauptthread anlegen

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

	DDRD |= RELAIS;
	DDRD |= IGBT;

	uart_init();

	adc_init(CH_TEMPERATURE); // Temperatur sensor
	adc_init(CH_VOLTAGE);     // Voltage
	adc_init(CH_CHARGE);      // Current +
	adc_init(CH_DISCHARGE);   // Current -

	cmd_init();

	i2c_init(100000); // 100kHz Speed

	simulator_init();
	supervisor_init();

	ePowerIst = ePowerOff;

	while(1)
	{
		if( ePowerSoll != ePowerIst)
		{
			if( ePowerSoll == ePowerFull )
			{
				PORTD |= IGBT;         // Switch IGBT on
				os_thread_sleep(50);   // Relais spark quenching
				PORTD |= RELAIS;       // Switch Relais on
			}
			else if( ePowerSoll == ePowerSave )
			{
				PORTD |= IGBT;         // Switch IGBT on
				PORTD &= ~RELAIS;      // Switch Relais off
			}
			else
			{
				PORTD &= ~RELAIS;      // Switch Relais off
				os_thread_sleep(200);  // Relais spark quenching
				PORTD &= ~IGBT;        // Switch IGBT off
				os_thread_sleep(5000); // Let IGBT cool down, in case we're still powered.
			}
			ePowerIst = ePowerSoll;
		}
		os_thread_yield();
	}
	return 0;
}
