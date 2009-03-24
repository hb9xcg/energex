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
#include "delay.h"
#include "protocol.h"
#include "battery.h"
#include "charge.h"
#include "data.h"

#define V              100UL
#define GRAD           100L
#define A              100L
#define Ah             100L
#define mA             10L
#define V_REF          2500UL     // [mV]
#define ADC_RESOLUTION 10


typedef enum
{
	eCheckVoltage,
	eCheckCurrent,
	eCheckMinTemp,
	eCheckMaxTemp,
	eCheckPower,
	eCheckNothing,
	eCheckInvalid
} ECheckState;


static EDriveState eDriveState = eConverterOff;
static EDriveState eLastState  = eConverterOff;
EPowerState ePowerIst;
int16 mediator_temperature;

static void wait_for_power(void);
static void mediator_check_power();
static void mediator_check_drive_voltage();
static void mediator_check_charge_voltage();
static void mediator_check_current();
static void mediator_check_min_charge_temperature();
static void mediator_check_max_charge_temperature();
static void mediator_check_min_drive_temperature();
static void mediator_check_max_drive_temperature();
static void mediator_limit_sym_current();
static void mediator_check_end_of_charge();
static void mediator_check_capacity();

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


	DDRB  |=  RELAIS; 		// Output
	DDRB  |=  IGBT; 		// Output
	DDRD  &= ~STOP_SWITCH; 		// Input
	PORTD |=  STOP_SWITCH; 		// Pullup enabled
	DDRD  |=  POWER_ONE_WIRE; 	// Output
	PORTD &= ~POWER_ONE_WIRE; 	// Power off

	DDRC  |=  LED_RED;   // Output
	DDRC  |=  LED_GREEN; // Output
	PORTC &= ~LED_RED;   // Switch off red led.
	PORTC &= ~LED_GREEN; // Switch off green led.

	uart_init();

	wait_for_power();

	PORTD |= POWER_ONE_WIRE; // Power on
	
//	adc_init(CH_TEMPERATURE); // Temperatur sensor
	adc_init(CH_VOLTAGE);     // Voltage
//	adc_init(CH_CHARGE);      // Current +
//	adc_init(CH_DISCHARGE);   // Current -

	battery_init();
	data_load();

	cmd_init();

	i2c_init(100000); // 100kHz Speed

	delay(10); // Give One wire power supply some time

	sensors_init();

	ePowerIst = ePowerOff;
	battery_info_set(BAT_REL_OPEN);    // Atomic update of battery info

	EICRA = (1<<ISC01) | (1<<ISC10); // Configure any change on pin INT0 for interrupt.
	EIMSK = (1<<INT0);               // Enable INT0 interrupt

	ePowerSoll = ePowerSave;

	while(1)
	{
		mediator_check_power();

		switch(eDriveState)
		{
		case 	eConverterOff: // InvOff           Umrichter aus
		case 	eConverterTest: // InvTest          Test-Modus
		case 	eConverterProg: // InvProg          Programm-Modus
		case 	eConverterIdle: // InvIdle          Umschaltungszustand vor dem Fahren
		case 	eBreakDown: // BreakDown        Fehler des Umrichters 
			break;
		case 	eDrive: // Drive            Fahren
			if (battery_info_get(BAT_FULL))
			{
				battery_info_clear(BAT_FULL);
				charge_set_capacity(280); // 2.8Ah
			}	
			break;
		case 	eReadyCharge: // ReadyCharge      warten auf Netzspannung
			break;
		case 	ePreCharge: // PreCharge        Vorladung
			if (eLastState==eReadyCharge)
			{
				charge_reset(); // Reset Ah counter
			}
		case 	eClosePCRelais: // CloseVRelais     Vorladerelais schliessen
			ePowerSoll = ePowerFull;
			break;
		case 	eICharge: // ICharge          I-Ladung
		case 	eUCharge: // UCharge          U-Ladung
			mediator_check_end_of_charge();
			mediator_check_capacity();
			break; 
		case 	eOpenBatRelais: // OpenBRelais      Batterierelais öffnen
			ePowerSoll = ePowerSave;	// can we open the relais while waiting for power?
			break;
		case 	ePostCharge: // PostCharge       Nachladung
			mediator_check_end_of_charge();
			mediator_check_capacity();
			break;
		case 	eCloseBatRelais: // CloseBRelais     Batterierelais schliessen
			ePowerSoll = ePowerFull;
			break;
		case 	eSymCharge: // SymmCharge       Symmetrierladung
		case 	eTrickleCharge: // TrickleCharge    Erhaltungsladung
		case 	eTrickleWait: // TrickleWait      Pause
		case 	eUnknown17: // (?)              (möglicherweise Erhaltungsladung einzelner Blöcke?)
		case 	eUnknown18: // (?)              (möglicherweise Erhaltungsladung einzelner Blöcke?)
		case 	eUnknown19: // (?)              (möglicherweise Erhaltungsladung einzelner Blöcke?)
		case 	eUnknown20:  // (?)
			break;
		default:
			TOGGLE_RED_LED;
		break;
		}
		eLastState = eDriveState;
		os_thread_sleep(100);
		//uart_write("Hallo\n\r", 7);
	}
	return 0;
}

void wait_for_power()
{
	PORTB |= IGBT;                             // Switch IGBT on

	while (!uart_data_available());            // Wait until Twike DC/DC converter delivers power.
}

void mediator_check_binfo(void)
{
	switch(eDriveState)
	{
		case 	eDrive: // Drive            Fahren
			mediator_check_min_drive_temperature();
			mediator_check_max_drive_temperature();
			mediator_check_drive_voltage();
			mediator_check_current();
			break;
		case 	eReadyCharge: 		// ReadyCharge      warten auf Netzspannung
		case	ePreCharge: 		// PreCharge        Vorladung
		case	eClosePCRelais: 	// CloseVRelais     Vorladerelais schliessen
		case	eICharge: 		// ICharge          I-Ladung
		case	eUCharge: 		// UCharge          U-Ladung
		case	eOpenBatRelais: 	// OpenBRelais      Batterierelais öffnen
		case	ePostCharge: 		// PostCharge       Nachladung
		case	eCloseBatRelais:	// CloseBRelais     Batterierelais schliessen
		case	eTrickleCharge:		// TrickleCharge    Erhaltungsladung
		case	eTrickleWait:		// TrickleWait      Pause
			mediator_check_min_charge_temperature();
			mediator_check_max_charge_temperature();
			mediator_check_charge_voltage();
			mediator_check_current();
			break;
		case 	eSymCharge:		// SymmCharge       Symmetrierladung
			mediator_limit_sym_current();
			break;
		default:
			break;
	}
}

void mediator_check_power()
{
	if( ePowerSoll != ePowerIst)
	{
		if( ePowerSoll == ePowerFull )
		{
			PORTB |= IGBT;             // Switch IGBT on
			PORTB |= RELAIS;           // Switch Relais on
			os_thread_sleep(300);      // Relais delay time
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
}

void mediator_check_drive_voltage()
{
	uint16_t voltage = battery_get_voltage();

	if (voltage < 310*V)
	{
		battery_info_set(VOLTAGE_TO_LO);
		battery_info_clear(VOLTAGE_TO_HI);
	}
	else if (voltage < 400*V)
	{
		battery_info_clear(VOLTAGE_TO_LO);
		battery_info_clear(VOLTAGE_TO_HI);
	}
	else
	{
		battery_info_clear(VOLTAGE_TO_LO);
		battery_info_set(VOLTAGE_TO_HI);
	}
}

void mediator_check_charge_voltage()
{
	uint16_t voltage = battery_get_voltage();

	if (voltage < 310*V)
	{
		battery_info_set(VOLTAGE_TO_LO);
		battery_info_clear(VOLTAGE_TO_HI);
	}
	else if (voltage < 435*V)
	{
		battery_info_clear(VOLTAGE_TO_LO);
		battery_info_clear(VOLTAGE_TO_HI);
	}
	else
	{
		battery_info_clear(VOLTAGE_TO_LO);
		battery_info_set(VOLTAGE_TO_HI);
	}
}

void mediator_check_current()
{
	int16_t current = charge_get_current();

	if (current > 280) // > 2.8A charge current
	{
		battery_info_set(CHARGE_CUR_TO_HI);
		battery_info_clear(DRIVE_CUR_TO_HI);
	}
	else if (current > -15*A) //  1A...-10A discharge current
	{
		battery_info_clear(CHARGE_CUR_TO_HI);
		battery_info_clear(DRIVE_CUR_TO_HI);
	}
	else //  > 10A discharge
	{
		battery_info_clear(CHARGE_CUR_TO_HI);
		battery_info_set(DRIVE_CUR_TO_HI);
	}
}

void mediator_check_capacity()
{
	int16_t charge = charge_get_capacity();

	if (charge > 4*Ah) // > 4Ah capacity
	{
		battery_info_set(BAT_FULL);
	}
}

void mediator_limit_sym_current()
{
	int16_t current = charge_get_current();

	if (current > 28) // > 280mA
	{
		battery_info_set(CHARGE_CUR_TO_HI);
	}
	else
	{
		battery_info_clear(CHARGE_CUR_TO_HI);
	}
}

void mediator_check_max_charge_temperature()
{
	int16_t bat_temp;

	sensors_get_max_temperatur(&bat_temp);

	if (bat_temp < 45*GRAD) // 0...45°C
	{
		battery_info_clear(CHARGE_TEMP_TO_HI);
	}
	else
	{
		battery_info_set(CHARGE_TEMP_TO_HI);
	}
}

void mediator_check_max_drive_temperature()
{
	int16_t bat_temp;

	sensors_get_max_temperatur(&bat_temp);

	if (bat_temp < 55*GRAD)
	{
		battery_info_clear(DRIVE_TEMP_TO_HI);
	}
	else // >55°C
	{
		battery_info_set(DRIVE_TEMP_TO_HI);
	}
}


void mediator_check_min_charge_temperature()
{
	int16_t bat_temp;

	sensors_get_min_temperatur(&bat_temp);

	if (bat_temp < 0*GRAD)
	{
		battery_info_set(CHARGE_TEMP_TO_LO);
	}
	else // >55°C
	{
		battery_info_clear(CHARGE_TEMP_TO_LO);
	}
}

void mediator_check_min_drive_temperature()
{
	int16_t bat_temp;

	sensors_get_min_temperatur(&bat_temp);

	if (bat_temp < -20*GRAD)
	{
		battery_info_set(DRIVE_TEMP_TO_LO);
	}
	else
	{
		battery_info_clear(DRIVE_TEMP_TO_LO);
	}
}

int16_t mediator_get_temperature(void)
{
	int32_t temperature;

	os_enterCS();
	temperature = mediator_temperature;
	os_exitCS();

	temperature  *= 10*V_REF;
	temperature >>= (ADC_RESOLUTION);
	temperature  -= 6000;

	return temperature;
}


void mediator_check_end_of_charge()
{
	static int16_t last_bat_temp = 45*GRAD;
	static uint32_t last_time;
	
	if (timer_ms_passed(&last_time, 120000) )
	{
		// every two minutes...
		int16_t bat_temp;
		sensors_get_avg_temperatur(&bat_temp);

		if (bat_temp - last_bat_temp > 1*GRAD)
		{
			battery_info_set(BAT_FULL);
		}
		last_bat_temp = bat_temp;
	}
}

void mediator_set_drive_state(EDriveState eState)
{
	eDriveState = eState;
}

EDriveState mediator_get_drive_state(void)
{
	return eDriveState;
}

/*!
 * Interrupt-Handler fuer den Interrupt 0. Trennt den das ganze Twike von der Batterie.
 */
ISR (INT0_vect)
{
	data_save();
	ePowerSoll = ePowerOff;
}
