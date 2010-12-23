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
 * @file 	plc.c
 * @brief 	Powerline controller.
 * @author 	Markus Walser (markus.walser@gmail.com)
 * @date 	20.10.2010
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include "plc.h"
#include <avr/eeprom.h>
#include "spi.h"
#include "adc.h"
#include "uart.h"
#include "timer.h"
#include "os_thread.h"
#include "command.h"
#include "delay.h"
#include "protocol.h"
#include "battery.h"
#include "data.h"
#include "error.h"
#include "idle.h"

#define V              100UL
#define GRAD           100L
#define A              100L
#define Ah             100L
#define mA             10L
#define V_REF          2500UL     // [mV]
#define ADC_RESOLUTION 10
#define OVER_SAMPLING  50


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

typedef enum
{
	eSilent,
	eAlive = 100, // 10 seconds timeout
	eForced
	
} ELiveness;


static EDriveState eDriveState = eConverterOff;
static EDriveState eLastState  = eConverterOff;
int16 plc_temperature;
ELiveness plc_alive = eAlive;
uint16_t plc_line_voltage;
int8_t plc_enable_sampling;

int main(void)
{
	PORTA=0; DDRA=0;		//Alles Eingang alles Null
	PORTB=0; DDRB=0;
	PORTC=0; DDRC=0;
	PORTD=0; DDRD=0;
	

	// Disable JTag to allow normal usage of Port C.		
	MCUCR = 1<< JTD;
	MCUCR = 1<< JTD;

	if (MCUSR & 1<<WDRF) {
		// We're comming from a soft reboot
		MCUSR &= ~(1<<WDRF);
	}
	wdt_disable();	// Watchdog aus!


	// main Hauptthread anlegen mit höchster Priorität.
	os_create_thread((uint8_t *)SP, NULL);		
	
	timer_2_init();
	
	/* Ist das ein Power on Reset? */
	if (MCUSR & (1<<PORF)) {
		MCUSR &= ~(1<<PORF);	// Bit loeschen
		delay(100);
		asm volatile("jmp 0");
	}

	delay(100);

	io_init();

	uart_init();

	spi_speed_t speed = SPI_SPEED_250KHZ;
	spi_master_init(speed);
	
	adc_init(CH_LINE_VOLTAGE);

	battery_init();

	data_load();

	cmd_init();

	idle_init();

	plc_enable_sampling = 1;

	while(1)
	{
		switch(eDriveState)
		{
		case 	eConverterOff: // InvOff           Umrichter aus
		case 	eConverterTest: // InvTest          Test-Modus
		case 	eConverterProg: // InvProg          Programm-Modus
		case 	eConverterIdle: // InvIdle          Umschaltungszustand vor dem Fahren
		case 	eBreakDown: // BreakDown        Fehler des Umrichters 
		case 	eDrive: // Drive            Fahren
			break;
		case 	eReadyCharge: // ReadyCharge      warten auf Netzspannung
			io_close_relais();
			break;
		case 	ePreCharge: // PreCharge        Vorladung
		case 	eClosePCRelais: // CloseVRelais     Vorladerelais schliessen
		case 	eICharge: // ICharge          I-Ladung
		case 	eUCharge: // UCharge          U-Ladung
		case 	eOpenBatRelais: // OpenBRelais      Batterierelais öffnen
		case 	ePostCharge: // PostCharge       Nachladung
		case 	eCloseBatRelais: // CloseBRelais     Batterierelais schliessen
			break;
		case 	eSymCharge: // SymmCharge       Symmetrierladung
		case 	eTrickleCharge: // TrickleCharge    Erhaltungsladung
		case 	eTrickleWait: // TrickleWait      Pause
			io_open_relais();
			break;
		case 	eUnknown17: // (?) (möglicherweise Erhaltungsladung einzelner Blöcke?)
		case 	eUnknown18: // (?) (möglicherweise Erhaltungsladung einzelner Blöcke?)
		case 	eUnknown19: // (?) (möglicherweise Erhaltungsladung einzelner Blöcke?)
		case 	eUnknown20:  // (?)
			break;
		default:
			error(ERROR_INVALID_DSTATE);
		break;
		}
		eLastState = eDriveState;
		os_thread_sleep(100);
//		uart_write("Hallo\n\r", 7);
//		io_toggle_red_led();
	}
	return 0;
}


void plc_check_binfo(void)
{
	switch(eDriveState)
	{
		case 	eDrive: // Drive            Fahren
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
			break;
		case 	eSymCharge:		// SymmCharge       Symmetrierladung
			break;
		default:
			break;
	}
}
	
void plc_busy(void)
{
	plc_alive = eAlive;
}

void plc_force_busy(int8_t on)
{
	if (on)
	{
		plc_alive = eForced;
	}
	else
	{
		plc_alive = eAlive;
	}
}

void plc_check_end_of_charge()
{
	if (battery_info_get(BAT_FULL))
	{
		io_open_relais();
	}
}

void plc_set_drive_state(EDriveState eState)
{
	eDriveState = eState;
}

EDriveState plc_get_drive_state(void)
{
	return eDriveState;
}

// gets called every 400us from the timer interrupt 
void plc_sample()
{
	static uint8_t idx;
	static uint16_t voltage_oversampling;
	static int16_t voltage_sample;

	voltage_oversampling += voltage_sample;
	
	if (++idx >= OVER_SAMPLING )
	{
		idx = 0;
		plc_line_voltage = voltage_oversampling;
		voltage_oversampling = 0;
		voltage_sample = 0;
	}

	adc_read_int( CH_LINE_VOLTAGE, &voltage_sample);
}

uint16_t plc_get_line_voltage(void)
{
	uint16_t line_voltage;

	os_enterCS();
	line_voltage = plc_line_voltage;
	os_exitCS();

	if (io_is_relais_closed())
	{
		line_voltage /= 134;
	}
	else
	{
		line_voltage /= 191;
	}

	return line_voltage;
}

