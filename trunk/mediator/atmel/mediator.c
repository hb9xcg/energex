/***************************************************************************
 *   Energex                                                               *
 *                                                                         *
 *   Copyright (C) 2008-2011 by Markus Walser                              *
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

#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include "mediator.h"
#include "i2c.h"
#include "spi.h"
#include "adc.h"
#include "uart.h"
#include "timer.h"
#include "os_thread.h"
#include "command.h"
#include "delay.h"
#include "protocol.h"
#include "battery.h"
#include "charge.h"
#include "data.h"
#include "ltc6802.h"
#include "error.h"
#include "io.h"
#include "gauge.h"
#ifdef BALANCER
	#include "ltc6802.h"
	#include "balancer.h"
#endif

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

typedef enum
{
	eSilent,
	eAlive = 100, // 10 seconds timeout
	eForced
	
} ELiveness;


static EDriveState eDriveState = eConverterOff;
static EDriveState eLastState  = eConverterOff;
EPowerState ePowerIst;
int16 mediator_temperature;
ELiveness mediator_alive = eAlive;
static uint8_t mediator_stop;

static void wait_for_power(void);
static void mediator_check_liveness(void);
static void mediator_check_power(void);
static void mediator_check_deep_discharge(void);
static void mediator_check_drive_voltage(void);
static void mediator_check_charge_voltage(void);
static void mediator_check_current(void);
static void mediator_check_min_charge_temperature(void);
static void mediator_check_max_charge_temperature(void);
static void mediator_check_min_drive_temperature(void);
static void mediator_check_max_drive_temperature(void);
static void mediator_limit_sym_current(void);
static void mediator_check_end_of_charge(void);
static void mediator_check_capacity(void);
static void mediator_check_max_cell_voltage(void);
static void mediator_update_statistics(void);
static void mediator_debounce_stop(void);
static void mediator_stop_pressed(void);
static void mediator_stop_released(void);
static void mediator_calibrate_gauge(void);

int main(void)
{
	io_init();

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

	uart_init();

//	spi_speed_t speed = SPI_SPEED_125KHZ;
	spi_speed_t speed = SPI_SPEED_250KHZ;
//	spi_speed_t speed = SPI_SPEED_1MHZ;
	spi_master_init(speed);
	
	adc_init();
	
	// Calibrate current ADC offset without any load
	adc_calibrate_offset(CH_CURRENT_1);
	adc_calibrate_offset(CH_CURRENT_10);
	adc_calibrate_offset(CH_CURRENT_200);

	
/*	power_all_disable();
	power_usart0_enable();
	set_sleep_mode(SLEEP_MODE_IDLE);
*/
	ePowerIst = ePowerSave;

	io_enable_igbt();              	// Power-up Twike DC/DC converter
	io_release_emergency();	       	// Power-up board computer

//	sleep_mode();

	wait_for_power();		// Wait for board computer communication
	
//	power_all_enable();
	
	// Calibrate voltage offset at normal voltage
	adc_calibrate_offset(CH_VOLTAGE_CALIB);

	io_enable_interface_power();
	
	battery_init();

	data_load();

	cmd_init();

	i2c_init(100000); // 100kHz Speed

	ltc_init();
	ltc_update_data();
	mediator_calibrate_gauge();

	balancer_init(); // Lowest thread priority

	battery_info_set(BAT_REL_OPEN);    // Atomic update of battery info


	ePowerSoll = ePowerSave;

	while(1)
	{
		mediator_check_liveness();
		mediator_check_power();

		switch(eDriveState)
		{
		case 	eConverterOff: // InvOff           Umrichter aus
			mediator_check_deep_discharge();
			break;
		case 	eConverterTest: // InvTest          Test-Modus
		case 	eConverterProg: // InvProg          Programm-Modus
		case 	eConverterIdle: // InvIdle          Umschaltungszustand vor dem Fahren
		case 	eBreakDown: // BreakDown        Fehler des Umrichters 
			break;
		case 	eDrive: // Drive            Fahren
			if (eLastState!=eDrive)
			{
				if (battery_info_get(BAT_FULL) ||
				    charge_get_capacity() > 2500)
				{
					battery_info_clear(BAT_FULL);
					mediator_calibrate_gauge();
				}
			}	
			break;
		case 	eReadyCharge: // ReadyCharge      warten auf Netzspannung
			break;
		case 	ePreCharge: // PreCharge        Vorladung
			if (eLastState==eReadyCharge)
			{
				mediator_update_statistics();
				charge_start(); // Reset Ah counter
				battery_info_clear(BAT_EMPTY);
			}
		case 	eClosePCRelais: // CloseVRelais     Vorladerelais schliessen
			ePowerSoll = ePowerFull;
			break;
		case 	eICharge: // ICharge          I-Ladung
			mediator_check_max_cell_voltage();
		case 	eUCharge: // UCharge          U-Ladung
			if (eDriveState == eUCharge &&
			     eLastState != eUCharge)
			{
// TODO Remove comment
//				balancer_set_state(BALANCER_ACTIVE);
			}
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
			io_toggle_red_led();
		break;
		}
		eLastState = eDriveState;
		os_thread_sleep(100);
		mediator_debounce_stop();
		//uart_write("Hallo\n\r", 7);
		//TOGGLE_GREEN_LED;
	}
	return 0;
}


void wait_for_power()
{
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

void mediator_calibrate_gauge(void)
{
	int16_t min, avg, max, capacity;
	
	ltc_get_voltage_min_avg_max(&min, &avg, &max);
	
	// Calculate initial capacity base on lowest cell voltage
	capacity = gauge_get_capacity(ltc_adc_voltage(min));
	charge_set_capacity(capacity);
	
	// Set also initial sym volages
	battery_set_parameter_value(SYM_SPANNUNG, BATTERY_1, ltc_adc_voltage(min)/10);
	battery_set_parameter_value(SYM_SPANNUNG, BATTERY_2, ltc_adc_voltage(avg)/10);
	battery_set_parameter_value(SYM_SPANNUNG, BATTERY_3, ltc_adc_voltage(max)/10);

	// and initial temperatures
	ltc_get_temperature_min_avg_max(&min, &avg, &max);
	battery_set_parameter_value(BATTERIE_TEMP, BATTERY_1, min);
	battery_set_parameter_value(BATTERIE_TEMP, BATTERY_2, avg);
	battery_set_parameter_value(BATTERIE_TEMP, BATTERY_3, max);
}
	
void mediator_busy(void)
{
	mediator_alive = eAlive;
}

void mediator_force_busy(int8_t on)
{
	if (on)
	{
		mediator_alive = eForced;
	}
	else
	{
		mediator_alive = eAlive;
	}
}

void mediator_check_liveness()
{
	if (mediator_alive == eSilent && ePowerSoll != ePowerOff)
	{
		io_set_red_led();
		data_save();
		balancer_set_state(BALANCER_STANDBY);
		ePowerSoll = ePowerOff;
		wdt_enable(WDTO_4S); // reboot into 'wait for power'. 
		io_clear_red_led();
	}
	else if (mediator_alive != eForced)
	{
		mediator_alive--;
	}
}

void mediator_check_power()
{
	if (ePowerSoll != ePowerIst)
	{
		if (ePowerSoll == ePowerFull)
		{
			io_enable_igbt();                  // Switch IGBT on

			io_close_relais();                 // Switch Relais on
			os_thread_sleep(300);              // Relais delay time
			battery_info_clear(BAT_REL_OPEN);  // Atomic update of battery info
		}
		else if (ePowerSoll == ePowerSave)
		{
			battery_info_set(BAT_REL_OPEN);    // Atomic update of battery info
			io_enable_igbt();                  // Switch IGBT on
			io_open_relais();                  // Switch Relais off				
		}
		else
		{
			battery_info_set(BAT_REL_OPEN);    // Atomic update of battery info
			io_open_relais();                  // Switch Relais off
			os_thread_sleep(500);              // Relais spark quenching
			io_disable_igbt();                 // Switch IGBT off
			os_thread_sleep(5000);             // Let IGBT cool down, in case we're still powered.
		}
		ePowerIst = ePowerSoll;
	}
}

void mediator_check_drive_voltage()
{
	uint16_t voltage = battery_get_voltage();

	if (voltage < 326*V)
	{
		ePowerSoll = ePowerOff;
	}
	else if (voltage < 336*V)
	{
		battery_info_set(BAT_EMPTY);
		battery_info_set(VOLTAGE_TO_LO);
		battery_info_clear(VOLTAGE_TO_HI);
	}
	else if (voltage < 375*V)
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

void mediator_check_deep_discharge()
{
	uint16_t voltage = battery_get_voltage();

	if (voltage < 326*V)
	{
		ePowerSoll = ePowerOff;
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
	else if (voltage < 375*V)
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
//		battery_info_set(CHARGE_CUR_TO_HI);
		battery_info_clear(CHARGE_CUR_TO_HI);
		battery_info_clear(DRIVE_CUR_TO_HI);
	}
	else if (current > -20*A) //  1A...-10A discharge current
	{
		battery_info_clear(CHARGE_CUR_TO_HI);
		battery_info_clear(DRIVE_CUR_TO_HI);
	}
	else //  > 20A discharge
	{
		battery_info_clear(CHARGE_CUR_TO_HI);
		battery_info_set(DRIVE_CUR_TO_HI);
	}
}

void mediator_check_capacity()
{
	int16_t charge = charge_get_capacity();

	if (charge > 26*Ah) // > 26Ah capacity
	{
		battery_info_set(BAT_FULL);
	}
}

void mediator_check_max_cell_voltage(void)
{
	if (battery_info_get(CHARGE_CUR_TO_HI))
	{
// TODO Remove comment
//		balancer_set_state(BALANCER_ACTIVE);
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

	bat_temp = ltc_get_temperature_max();

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

	bat_temp = ltc_get_temperature_max();

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

	bat_temp = ltc_get_temperature_min();

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

	bat_temp = ltc_get_temperature_min();

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
	if ( eDriveState == eUCharge &&
	     charge_get_current() < 125)
	{
		battery_info_set(BAT_FULL);
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


void mediator_update_statistics()
{
	if (battery_get_voltage() < TIEFENTLADE_SPANNUNG)
	{
		data_deep_discharge_cycles++;
	}

	int16_t discharged_capacity = data_nominal_capacity - charge_get_capacity();
	if (discharged_capacity > data_max_capacity)
	{
		data_max_capacity = discharged_capacity;
	}
	if (discharged_capacity < data_min_capacity)
	{
		data_min_capacity = discharged_capacity;
	}
	
	data_charge_cycles++;

	int16_t gap = data_nominal_capacity / 10;
	int16_t actual_capacity = charge_get_capacity();
	int16_t border = data_nominal_capacity-gap;

	for (int8_t stat=0; stat<12; stat++)
	{
		if (actual_capacity >= border)
		{
			data_stat.cycles[stat]++;
			return;
		}
		border -= gap;
	}
	data_stat.cycles_over_110++;
}

void mediator_cell_limit_reached(void)
{
	if (eDriveState==eDrive)
	{
		battery_info_set(BAT_EMPTY);
	}
	else
	{
		battery_info_set(CHARGE_CUR_TO_HI);
	}
}

void mediator_cell_limit_ok(void)
{
	if (eDriveState!=eDrive)
	{
		battery_info_clear(CHARGE_CUR_TO_HI);
	}
}

void mediator_debounce_stop(void)
{
	static uint8_t last_level;

	mediator_stop <<= 1;
	if (io_get_stop())
	{
		mediator_stop |= 0x1;;
	}

	mediator_stop &= 0xf;

	if (mediator_stop == 0xf)
	{
		if (last_level != 1)
		{
			last_level = 1;
			mediator_stop_released();
		}

	}
	else if (mediator_stop == 0)
	{
		if (last_level != 0)
		{
			last_level = 0;
			mediator_stop_pressed();
		}
	}
}

void mediator_stop_released(void)
{
}

void mediator_stop_pressed(void)
{
	io_set_red_led();
	data_save();
	io_clear_red_led();
	
	io_raise_emergency();

	ePowerSoll = ePowerSave;
	io_disable_interface_power();
	
	mediator_set_drive_state(eConverterOff);
}

