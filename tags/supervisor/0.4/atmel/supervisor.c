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
 * @file 	supervisor.c
 * @brief 	Contains main cell supervisor code.
 * @author 	Markus Walser (markus.walser@gmail.com)
 * @date 	04.03.2008
 */

#include <avr/io.h>
#include <stdint.h> //definiert den Datentyp uint8_t
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/eeprom.h> 

//#include "i2c-slave.h"
#include "adc.h"
#include "led.h"
#include "timer.h"
#include "debug.h"
#include "USI_TWI_Slave.h"

/* EEPROM */
#ifndef EEMEM
#define EEMEM __attribute__ ((section (".eeprom")))
#endif

// Defines
#define DEFAULT_I2C_ADDRESS 39

#define DEBUG	0
#define SLEEP	0

#if DEBUG
  #define SetD(x) PORTD &= ~(1<<(x))
  #define ClrD(x) PORTD |=  (1<<(x))
#else
  #define SetD(x)
  #define ClrD(x)
#endif


uint16_t callibration = 20;

/* Byte: I2C address */
uint8_t eeI2CAddress EEMEM = DEFAULT_I2C_ADDRESS;
/* Wort ADC calibration */
uint16_t eeCalibration EEMEM = 610;

void female(void);
void male(void);


void
initialize(void)
{
	uint8_t address;
	

    DDRB  |= 0x08;  // enable output of pin 3 on Port B
    DDRB  |= 0x10;  // enable output of pin 4 on Port B
    DDRB  |= 0x20;  // enable output of pin 5 on Port B
	DDRB  |= 0x40;  // enable output of pin 6 on Port B (Power)
    RED(OFF);       // clear pin 3 on port B
    GREEN(OFF);     // clear pin 4 on port B
    YELLOW(OFF);    // clear pin 5 on port B

	POWER(ON);		// Hold Power supply switched on

    DDRA   = 0x00;	// configure port A as input
    PORTA  = 0x00;  // disable pullups

	DDRB  |= 0x20;  // enable output of bit 5 on Port B (I2C)
	I2C(ON);        // Power on I2C Optocoupler
	

	eeprom_busy_wait();
	callibration = eeprom_read_word( &eeCalibration );
	eeprom_busy_wait();
	address = eeprom_read_byte( &eeI2CAddress );

	if( address == 0xff)
	{
		address = DEFAULT_I2C_ADDRESS;
//		GREEN(ON); at the moment this led is required for debugging
	}
	
    USI_TWI_Slave_Initialise(address);

	timer_initialize();

#if SLEEP
    MCUCR |= 1<<SE | 1<<SM1|1<<SM0;	// power-down when sleeping
#endif

	sei();

	RED(OFF);
}




int main(void)
{
	char thousands, hunderts, decimals, digits;
	char cmd;
	uint32_t voltage;

    initialize();


	for(;;)
	{
#if SLEEP
		// Disable watchdog timer
		wdt_disable();

		/*
		 * Cannot use the avr-libc code, as there exists a race condition when the 
		 * interrupt occurs between the idle test and the sleep.  Since the instruction
		 * after the SEI instruction is always executed before an interrupt, this code 
		 * is safe however.
		 */
		cli();
		if (i2c_idle()) {
		    asm volatile ("sei\n\tsleep");
		}
		sei();
	
		// Enable watchdog timer
		wdt_enable(WDTO_2S);
#endif

		if( USI_TWI_Data_In_Receive_Buffer() )
		{
			cmd = USI_TWI_Receive_Byte();

			switch( cmd )
			{
				
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
				{
					watchdog = 0; // reset watchdog
					load = cmd - '0';
				}
				break;
				case 'a':
				{
					watchdog = 0; // reset watchdog
					load = 10;
				}
				break;
				case 'c':
				{
					watchdog = 0; // reset watchdog
					uint32_t voltage = readADC(0);
					callibration = 4200000 / voltage;
					eeprom_busy_wait();
					eeprom_write_word( &eeCalibration, callibration);
				}
				break;
				case 's':
				{
					watchdog = 0; // reset watchdog
					char byte = USI_TWI_Receive_Byte();
					eeprom_busy_wait();
					eeprom_write_byte( &eeI2CAddress, byte);
				}
				break;
				case 't':
				{
					watchdog = 0; // reset watchdog
					voltage = readADC(4);
					voltage *= callibration; // [1uV]
					voltage /= 2000; // [mV]
					voltage -= 600;  // eliminate 600mV LM61 DC Offset
					voltage /= 10;   // Round to °C
	
					if( voltage < 0 )
					{
						USI_TWI_Transmit_Byte('-');
						voltage *= -1;
					}
					else
					{
						USI_TWI_Transmit_Byte('+');
					}
					thousands = voltage/1000;
					hunderts  = (voltage-1000*thousands)/100;
					decimals  = (voltage-1000*thousands-100*hunderts)/10;
					digits    = voltage-1000*thousands-100*hunderts-10*decimals;
					
					USI_TWI_Transmit_Byte(decimals+48);
					USI_TWI_Transmit_Byte(digits+48);
					USI_TWI_Transmit_Byte('\'');
					USI_TWI_Transmit_Byte('C');
				}
				break;
				case 'u':
				{
					watchdog = 0; // reset watchdog
					voltage = readADC(0);
					voltage *= callibration; // [1uV]
					voltage/=1000; // [mV]
					thousands = voltage/1000;
					hunderts  = (voltage-1000*thousands)/100;
					decimals  = (voltage-1000*thousands-100*hunderts)/10;
					digits    = voltage-1000*thousands-100*hunderts-10*decimals;
					
					USI_TWI_Transmit_Byte(thousands+48);
					USI_TWI_Transmit_Byte(hunderts+48);
					USI_TWI_Transmit_Byte(decimals+48);
					USI_TWI_Transmit_Byte(digits+48);
					USI_TWI_Transmit_Byte('m');
					USI_TWI_Transmit_Byte('V');
				}
				break;
				default:
				break;
			}
		}
	}
	
	return 0;
}


// Wakeup line activated
ISR(INT0_vect)
{
	int i;
	for(i=0;i<3;i++)
	{
		RED(1);
		for(i=0;i<20000;i++) asm("nop");
		RED(0);
	}
}

