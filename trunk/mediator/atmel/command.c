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
 * @file 	command.c
 * @brief 	User debug prompt.
 * @author 	Markus Walser (markus.walser@gmail.com)
 * @date 	11.02.08
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h> 
#include <avr/io.h>
#include <avr/wdt.h> 
#include <avr/pgmspace.h>



#include "command.h"
#include "os_thread.h"
#include "uart.h"
#include "adc.h"
#include "charge.h"
#include "battery.h"
#include "protocol.h"
#include "ow.h"
#include "sensors.h"
#include "data.h"
#include "ntc.h"
#include "ko.h"
#ifdef BALANCER
	#include "ltc6802.h"
	#include "balancer.h"
#endif
#include "ds1307.h"
#include "io.h"

#define COMMAND_STACK_SIZE 384
#define CMD_LINE   128
#define DEBUG(str)  uart_write((uint8_t*)(str), sizeof(str) );

#define FRAME 		0x10
#define WELCOME		0x11

typedef enum
{
	eDebug,
	eTwike
} EMode;

uint8_t cmd_stack[COMMAND_STACK_SIZE];
static Tcb_t * cmd_thread;
static char cmd_line[CMD_LINE];

static EMode eMode = eTwike;

static void cmd_dispatcher(void);
static void cmd_receive_byte(uint8_t character);
static void cmd_process( const char* cmd);
static void cmd_help(void);
static void cmd_temperatur(void);
static void cmd_voltage(void);
static void cmd_current(void);
static void cmd_ko(void);
static void cmd_capacity(void);
static void cmd_reset(void);
static void cmd_offset(void);
static void cmd_reboot(void);
static void cmd_power(const char* cmd);
static void cmd_onewire(const char* cmd);
static void cmd_sensor(const char* cmd);
static void cmd_statistics(const char* cmd);
static void cmd_eeprom(const char* cmd);
static void cmd_stack_memory(void);
static void cmd_cell(void);
static void cmd_flash(void);
static void cmd_flush(void);
static void cmd_supervisor_temperature(void);
static void cmd_load(void);
static void cmd_date(void);
static void cmd_alpha(const char* cmd);
static void cmd_welcome(void);
static void cmd_binfo(void);

static void cmd_print_signed_float(int16_t value);
static void cmd_print_unsigned_float(uint16_t value);
static void cmd_print_signed_fix(int16_t value);
static void cmd_print_unsigned_fix(uint16_t value);


void cmd_init(void)
{
	memset(cmd_stack, 0xb6, sizeof(cmd_stack));
	cmd_thread = os_create_thread((uint8_t *)&cmd_stack[COMMAND_STACK_SIZE-1], cmd_dispatcher );	// Command thread anlegen
}

uint16_t cmd_get_max_stack_usage(void)
{
	uint16_t stack_idx = 0;
	while(stack_idx < COMMAND_STACK_SIZE)
	{
		if (cmd_stack[stack_idx] != 0xb6)
		{
			break;
		}
		stack_idx++;
	}
	return COMMAND_STACK_SIZE-stack_idx;
}


void cmd_dispatcher(void)
{
	uint8_t lastCharacter=0, character;	

	os_thread_sleep(100);
	
	for(;;)
	{
		if (uart_data_available() > 0)
		{
			uart_read( &character, sizeof(character) );

			
			// Check debug escape sequence "^pd"
			if (lastCharacter==FRAME)
			{
				if (character=='d') 
				{
					eMode = eDebug;
					mediator_force_busy(1);
					character = WELCOME;
				}
				else if(character==FRAME)
				{
					lastCharacter = 0;
				}
				else
				{
					lastCharacter = character;
				}
			}
			else
			{
				lastCharacter = character;
			}

			// Dispatch received byte
			if (eMode == eTwike)
			{
				mediator_busy();
				protocol_receive_byte(character);
			}
			else
			{
				if(character==FRAME)
				{
					eMode = eTwike;
					mediator_force_busy(0);
					protocol_receive_byte(character);
				}
				else
				{
					cmd_receive_byte(character);
				}
			}
		}
		else
		{
			os_thread_sleep(8);
		}
	}
}

void cmd_receive_byte(uint8_t character)
{
	static uint8_t lineIdx;

	uart_write( &character, sizeof(character) );

	if (character == 0x8 )
	{
		if(lineIdx>0)
			lineIdx--;
	}
	else if (character == '\n')
	{
	}
	else if (character == '\r')
	{
		// Line completed
		DEBUG("\r\n");
		cmd_line[lineIdx] = '\0';
		cmd_process((char*)cmd_line);
		strcpy_P(cmd_line, PSTR("\r\nroot@mediator:~> "));
		cmd_flush();
		lineIdx=0;
		cmd_line[lineIdx] = '\0';
	}
	else if (character == '\0')
	{
		io_toggle_red_led();
	}
	else
	{
		cmd_line[lineIdx] = character;
		lineIdx++;
	}
}

void cmd_process( const char* cmd )
{	
	switch( cmd[0] )
	{
		case WELCOME:
			cmd_welcome();
		break;
		case 'a':
			cmd_alpha(cmd);
		break;
		case 'b':
			cmd_reboot();
		break;
		case 'c':
			cmd_cell();
		break;
		case 'd':
			cmd_date();
		break;
		case 'e':
			cmd_eeprom(cmd);
		break;
		case 'f':
			cmd_flash();
		break;
		case 'i':
			cmd_current();
		break;
		case 'k':
			cmd_ko();
		break;
		case 'l':
			cmd_load();
		break;
		case 'm':
			cmd_stack_memory();
		break;
		case 'o':
			cmd_offset();
		break;
		case 'p':
			cmd_power(cmd);
		break;
		case 'q':
			cmd_capacity();
		break;
		case 'r':
			cmd_reset();
		break;
		case 's':
			cmd_statistics(cmd);
		break;
		case 't':
			cmd_supervisor_temperature();
		break;
		case 'u':
			cmd_voltage();
		break;
		case 'w':
			cmd_onewire(cmd);
		break;
		case 'x':
			eMode = eTwike;
			mediator_force_busy(0);
		break;
		case 'B':
			cmd_binfo();
		break;
		case '?':
			cmd_help();
		break;
		default:
			strcpy_P(cmd_line, PSTR("<Unknown command, try ? for help."));
	}
	cmd_flush();
}

void cmd_help(void)
{
	strcpy_P(cmd_line, PSTR("\n\rAvailable commands:\n\r"));
	cmd_flush();
	strcpy_P(cmd_line, PSTR("b:\tReboot\n\r"));
	cmd_flush();
	strcpy_P(cmd_line, PSTR("B:\tBattery info\n\r"));
	cmd_flush();
	strcpy_P(cmd_line, PSTR("c:\tCell voltages\n\r"));
	cmd_flush();
	strcpy_P(cmd_line, PSTR("d:\tDate and time\n\r"));
	cmd_flush();
	strcpy_P(cmd_line, PSTR("i:\tCurrent\n\r"));
	cmd_flush();
	strcpy_P(cmd_line, PSTR("m\tMemory info\n\r"));
	cmd_flush();
	strcpy_P(cmd_line, PSTR("p:\tPower state {full|save|off}\n\r"));
	cmd_flush();
	strcpy_P(cmd_line, PSTR("q:\tCapacity\n\r"));
	cmd_flush();
	strcpy_P(cmd_line, PSTR("r:\tReset\n\r"));
	cmd_flush();
	strcpy_P(cmd_line, PSTR("s:\tStatistics\n\r"));
	cmd_flush();
	strcpy_P(cmd_line, PSTR("t:\tTemperatur\n\r"));
	cmd_flush();
	strcpy_P(cmd_line, PSTR("u:\tVoltage\n\r"));
	cmd_flush();
	strcpy_P(cmd_line, PSTR("x:\tExit\n\r"));
	cmd_flush();
	strcpy_P(cmd_line, PSTR("?:\tHelp\n\r"));
}

void cmd_flush(void)
{
	uart_write( (uint8_t*)cmd_line, strlen(cmd_line) );
	uart_flush();
}

void cmd_eeprom(const char* cmd)
{
	if (strstr( cmd, "load") )
	{
		data_load();	
		strcpy_P(cmd_line, PSTR("Persistent data loaded."));
	}
	else if (strstr( cmd, "save") )
	{
		data_save();
		strcpy_P(cmd_line, PSTR("Persistent data saved."));
	}
	else if (strstr( cmd, "inct") )
	{
		uint16_t newCapacity;
		newCapacity = charge_get_capacity();
		newCapacity++;
		charge_set_capacity(newCapacity);
	}
}

void cmd_reboot(void)
{
	data_save();
	wdt_enable(WDTO_15MS);
}

void cmd_temperatur(void)
{
	int16_t temperature;

	temperature = mediator_get_temperature();

	strcpy_P( cmd_line, PSTR("Mediator temperatur: "));
	cmd_print_signed_float(temperature);
	strcpy_P( cmd_line, PSTR("°C"));
}


void cmd_voltage(void)
{
//
//	int16_t voltage = adc_read_polled(CH_VOLTAGE);
//	        (-)  (+)
/*	int16_t adc2_adc1 = adc_read_polled(0x19);
	int16_t adc2_adc0 = adc_read_polled(0x18);
	int16_t adc1_adc0 = adc_read_polled(0x10);
	int16_t adc1_adc1 = adc_read_polled(0x11);

	sprintf_P(cmd_line, 
		  PSTR("adc2_adc1=%d\r\nadc2_adc0=%d\r\nadc1_adc0=%d\r\nadc1_adc1=%d"),
		  adc2_adc1, adc2_adc0, adc1_adc0, adc1_adc1);

	return;
*/	
	uint16_t voltage;
	voltage = battery_get_voltage();	
	strcpy_P( cmd_line, PSTR("Mediator voltage: "));
	cmd_print_unsigned_float(voltage);	
	strcat_P( cmd_line, PSTR("V"));
}

void cmd_current(void)
{
	int16_t current;

	current = charge_get_current();

//	int16_t adc2_adc3_200 = adc_read_polled(0xf);

	strcpy_P( cmd_line, PSTR("Mediator current: "));
	cmd_print_signed_float(current);
	strcat_P( cmd_line, PSTR("A"));
}

void cmd_offset(void)
{
	int16_t offset;

	offset = adc_get_offset(CH_VOLTAGE);
	strcpy_P( cmd_line, PSTR("Mediator ADC voltage offset: "));
	cmd_print_signed_fix(offset);
	strcat_P(cmd_line, PSTR("\n\r"));
	cmd_flush();
	
	offset = adc_get_offset(CH_CURRENT_1);
	strcpy_P( cmd_line, PSTR("Mediator ADC current 1 offset: "));
	cmd_print_signed_fix(offset);
	strcat_P(cmd_line, PSTR("\n\r"));
	cmd_flush();

	offset = adc_get_offset(CH_CURRENT_10);
	strcpy_P( cmd_line, PSTR("Mediator ADC current 10 offset: "));
	cmd_print_signed_fix(offset);
	strcat_P(cmd_line, PSTR("\n\r"));
	cmd_flush();

	offset = adc_get_offset(CH_CURRENT_200);
	strcpy_P( cmd_line, PSTR("Mediator ADC current 200 offset: "));
	cmd_print_signed_fix(offset);
}

void cmd_capacity(void)
{
	int16_t capacity;

   	capacity = charge_get_capacity();
	strcpy_P( cmd_line, PSTR("Actual capacity:  "));
	cmd_print_signed_float(capacity);
	strcat_P( cmd_line, PSTR("Ah\n\r"));
	cmd_flush();

	strcpy_P( cmd_line, PSTR("Maximal capacity: "));
	cmd_print_signed_float(data_max_capacity);
	strcat_P( cmd_line, PSTR("Ah\n\r"));
	cmd_flush();

	strcpy_P( cmd_line, PSTR("Nominal capacity: "));
	cmd_print_signed_float(data_nominal_capacity);
	strcat_P( cmd_line, PSTR("Ah\n\r"));
	cmd_flush();

	strcpy_P( cmd_line, PSTR("Minimal capacity: "));
	cmd_print_signed_float(data_min_capacity);
	strcat_P( cmd_line, PSTR("Ah\n\r"));
	cmd_flush();

   	capacity = charge_get_total_charge();
	strcpy_P( cmd_line, PSTR("Total charge:     "));
	cmd_print_signed_fix(capacity);
	strcat_P( cmd_line, PSTR("Ah\n\r"));
	cmd_flush();

   	capacity = charge_get_total_discharge();
	strcpy_P( cmd_line, PSTR("Total discharge:  "));
	cmd_print_signed_fix(capacity);
	strcat_P( cmd_line, PSTR("Ah\n\r"));
	cmd_flush();

	cmd_line[0] = '\0';
}

void cmd_reset(void)
{
	charge_reset();
	data_deep_discharge_cycles = 0;
	data_charge_cycles = 0;
	data_nominal_capacity = 2500;
	data_min_capacity = 9999;
	data_max_capacity = 0;
	memset(data_stat.cycles, 0, 24);

	strcpy_P( cmd_line, PSTR("Mediator resetted capacity to 0mAh\r\n"));
	cmd_flush();

	ePowerSoll = ePowerFull;
	os_thread_sleep(500);

	sprintf_P(cmd_line, PSTR("Old calibration value: %u\r\n"), data_voltage_calibration);
	cmd_flush();

	uint32_t total=0;
	uint8_t i;

        for (i=0; i<BALANCER_NBR_OF_CELLS; i++)
        {
        	total += ltc_adc_voltage(ltc_get_voltage(i));
	}

	total /= 10;
	battery_calibrate(total);
	sprintf_P(cmd_line, PSTR("New calibration value: %u\r\n"), data_voltage_calibration);
	ePowerSoll = ePowerSave;
}

static void cmd_sensor(const char* cmd)
{
	int8_t nbr_of_sensors;
	uint8_t idx, serial[8];
	int16_t temp;

	strcpy_P(cmd_line, PSTR("device serial number      temperatur\n\r"));
	cmd_flush();
	
	nbr_of_sensors = sensors_get_nbr_of_devices();
	for(idx=0; idx<nbr_of_sensors; idx++)
	{
		temp = 0;
		sensors_get_temperatur( idx, &temp);
		sensors_get_serial( idx, serial);
		sprintf_P( cmd_line, PSTR("%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x      %d.%01d°C\r\n"),
				serial[0], 
				serial[1], 
				serial[2],
				serial[3],
				serial[4],
				serial[5],
				serial[6],
				serial[7],
				temp/100,
				temp%100);

		cmd_flush();
		uart_flush();
	}
	strcpy_P(cmd_line, PSTR("===================================="));
}

static void cmd_statistics(const char* cmd)
{
	strcpy_P(cmd_line, PSTR("\n\r<10%\t"));
	cmd_print_unsigned_fix(data_stat.cycles_under_10);
	cmd_flush();
	strcpy_P(cmd_line, PSTR("\n\r>10%\t"));
	cmd_print_unsigned_fix(data_stat.cycles_over_10);
	cmd_flush();
	strcpy_P(cmd_line, PSTR("\n\r>20%\t"));
	cmd_print_unsigned_fix(data_stat.cycles_over_20);
	cmd_flush();
	strcpy_P(cmd_line, PSTR("\n\r>30%\t"));
	cmd_print_unsigned_fix(data_stat.cycles_over_30);
	cmd_flush();
	strcpy_P(cmd_line, PSTR("\n\r>40%\t"));
	cmd_print_unsigned_fix(data_stat.cycles_over_40);
	cmd_flush();
	strcpy_P(cmd_line, PSTR("\n\r>50%\t"));
	cmd_print_unsigned_fix(data_stat.cycles_over_50);
	cmd_flush();
	strcpy_P(cmd_line, PSTR("\n\r>60%\t"));
	cmd_print_unsigned_fix(data_stat.cycles_over_60);
	cmd_flush();
	strcpy_P(cmd_line, PSTR("\n\r>70%\t"));
	cmd_print_unsigned_fix(data_stat.cycles_over_70);
	cmd_flush();
	strcpy_P(cmd_line, PSTR("\n\r>80%\t"));
	cmd_print_unsigned_fix(data_stat.cycles_over_80);
	cmd_flush();
	strcpy_P(cmd_line, PSTR("\n\r>90%\t"));
	cmd_print_unsigned_fix(data_stat.cycles_over_90);
	cmd_flush();
	strcpy_P(cmd_line, PSTR("\n\r>100%\t"));
	cmd_print_unsigned_fix(data_stat.cycles_over_100);
	cmd_flush();
	strcpy_P(cmd_line, PSTR("\n\r>110%\t"));
	cmd_print_unsigned_fix(data_stat.cycles_over_110);
	cmd_flush();

	strcpy_P(cmd_line, PSTR("\n\r\n\rCharge cycles:\t\t"));
	cmd_print_unsigned_fix(data_charge_cycles);
	cmd_flush();
	strcpy_P(cmd_line, PSTR("\n\rDeep discharge cylces:\t"));
	cmd_print_unsigned_fix(data_deep_discharge_cycles);
	cmd_flush();
	
	sprintf_P(cmd_line, PSTR("\n\rDaisy chain BER:\t0.%04d\r\n"), data_spi_ber);

}

void cmd_power(const char* cmd)
{
	if( strstr( cmd, "off") )
	{
		ePowerSoll = ePowerOff;
		strcpy_P(cmd_line, PSTR("Switched to power state \'off\'"));
	}
	else if( strstr( cmd, "full") )
	{
		ePowerSoll = ePowerFull;
		strcpy_P(cmd_line, PSTR("Switched to power state \'full\'"));
	}
	else if( strstr( cmd, "save") )
	{
		ePowerSoll = ePowerSave;
		strcpy_P(cmd_line, PSTR("Switched to power state \'save\'"));
	}
	else
	{
		strcpy_P(cmd_line, PSTR("Current state is: "));
		switch(ePowerSoll)
		{
		case ePowerOff:  strcat_P(cmd_line, PSTR("Off\n\r"));
				 break;
		case ePowerSave: strcat_P(cmd_line, PSTR("Save\n\r"));
				 break;
		case ePowerFull: strcat_P(cmd_line, PSTR("Full\n\r"));
				 break;
		default:         strcat_P(cmd_line, PSTR("Unknown\n\r"));
				 break;
		}
		strcat_P(cmd_line, PSTR("Use \'full\', \'save\' or \'off\' as argument"));
	}
}


void cmd_onewire(const char* cmd)
{
	int8_t result;
	if( strstr( cmd, "restart") )
	{
		result = ow_restart();
		sprintf_P( cmd_line, PSTR("ow_restart returned %d"), result);
	}
	else if (strstr( cmd, "reset") )
	{
		result = ow_reset();
		sprintf_P( cmd_line, PSTR("ow_reset returned %d"), result);
	}
	else if (strstr( cmd, "search"))
	{
		ow_reset();

		uint8_t last_device = 0;
		uint8_t snum[8];
		uint8_t nbr_of_dev = 0;

		for (;;)
		{
			io_set_green_led();

			result = ow_search( 0, &last_device, snum);
			sprintf_P( cmd_line, PSTR("one wire dev: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\r\n"),
				snum[0], snum[1], snum[2], snum[3], snum[4], snum[5], snum[6], snum[7]);
			cmd_flush();
			
			io_clear_green_led();

			if( result == 0) {
				nbr_of_dev++;
			}
			
			if (!last_device) {
				os_thread_sleep(1280);
			} else {
				break;
			}
		}
		
		sprintf_P( cmd_line, PSTR("ow_search found %d one wire device%s"), nbr_of_dev, nbr_of_dev>1 ? "s" : "");
	}
	else
	{
		strcpy_P(cmd_line, PSTR("Use \'restart\', \'reset\' or \'search\' as argument"));
	}
}

void cmd_stack_memory(void)
{
	uint16_t result = cmd_get_max_stack_usage();
	
	strcpy_P(cmd_line, PSTR("command_thread's max stack usage: "));
	cmd_print_unsigned_fix(result);
	strcat_P(cmd_line, PSTR(" bytes\r\n"));
	cmd_flush();

	result = balancer_get_max_stack_usage();
	strcpy_P(cmd_line, PSTR("balancer_thread's max stack usage: "));
	cmd_print_unsigned_fix(result);
	strcat_P(cmd_line, PSTR(" bytes\r\n"));
}

void cmd_flash(void)
{
#ifdef BALANCER
	balancer_set_state(BALANCER_OFF);
	os_thread_sleep(750);

	for (uint8_t flash=0; flash<12; flash++)
	{
		for (uint8_t i=0; i<BALANCER_NBR_OF_CELLS; i++)
		{
			ltc_set_load(i, 1);
		}
		
		ltc_write_config();
		os_thread_sleep(500);
		io_toggle_green_led();
		
		for (uint8_t i=0; i<BALANCER_NBR_OF_CELLS; i++)
		{
			ltc_set_load(i, 0);
		}
		
		ltc_write_config();
		os_thread_sleep(500);
	}
	balancer_set_state(BALANCER_SURVEILLANCE);
#endif
	strcpy_P(cmd_line, PSTR("finished"));
}

void cmd_cell(void)
{
	int8_t i;
		
	sprintf_P(cmd_line, PSTR(" Cell | Block A | Block B\r\n"));
	cmd_flush();

	uint32_t totalA=0, totalB=0;

	for (i=0; i<(BALANCER_NBR_OF_CELLS>>1); i++)
	{
		uint16_t voltageA = ltc_get_voltage(i);
		uint16_t voltageB = ltc_get_voltage(i+(BALANCER_NBR_OF_CELLS>>1));
		voltageA = ltc_adc_voltage(voltageA);
		voltageB = ltc_adc_voltage(voltageB);
		sprintf_P(cmd_line, PSTR("%5d | %5dmV | %5dmV\r\n"), 
				i, voltageA, voltageB);
		cmd_flush();

		totalA += voltageA;
		totalB += voltageB;
	}
	sprintf_P(cmd_line, PSTR("=========================\r\n"));
	cmd_flush();

	uint16_t mV, V;
	V  = totalA / 1000;
	mV = totalA % 1000;

	sprintf_P(cmd_line, PSTR("Total | %3d.%02dV "), 
			V, mV/10); 
	
	cmd_flush();

	V  = totalB / 1000;
	mV = totalB % 1000;

	sprintf_P(cmd_line, PSTR("| %3d.%02dV\r\n"), 
			V, mV/10);
	cmd_flush();

//	uint16_t total = totalA/10 + totalB/10;
//	battery_calibrate(total);
//
	int16_t min, avg, max;
	ltc_get_voltage_min_avg_max(&min, &avg, &max);
	
	sprintf_P(cmd_line, 
		  PSTR("\r\nMaximum = %dmV\r\nAverage = %dmV\r\nMinimum = %dmV\r\n"), 
		  ltc_adc_voltage(max), ltc_adc_voltage(avg), ltc_adc_voltage(min));

	cmd_flush();

	cmd_line[0] = 0;
}

void cmd_supervisor_temperature()
{
	uint8_t i;
	int16_t tempA=0, tempB=0;
	
	sprintf_P(cmd_line, PSTR(" Sensor | Block A | Block B\r\n"));
	cmd_flush();

	for (i=0; i<LTC_STACK_SIZE; i++)
	{
		tempA = ltc_get_external_temperature(i);
		tempB = ltc_get_external_temperature(i+LTC_STACK_SIZE);

		sprintf_P(cmd_line, PSTR("%7d | %4d.%02d | %4d.%02d\r\n"), 
				i, tempA/100, tempA%100, tempB/100, tempB%100);
		cmd_flush();
	}
	sprintf_P(cmd_line, PSTR("===========================\r\n"));

	cmd_flush();

	int16_t min, avg, max;
	ltc_get_temperature_min_avg_max(&min, &avg, &max);
	
	sprintf_P(cmd_line, PSTR("\r\nMaximum = %d.%02d"), max/100, max%100);
	cmd_flush();
	sprintf_P(cmd_line, PSTR("\r\nAverage = %d.%02d"), avg/100, avg%100);
	cmd_flush();
	sprintf_P(cmd_line, PSTR("\r\nMinimum = %d.%02d"), min/100, min%100);
	cmd_flush();

	strcpy_P(cmd_line, PSTR("\r\n"));
}

void cmd_ko(void)
{
	ko_print();
}

void cmd_load(void)
{
	static uint8_t on;

	on ^= 1;
#ifdef BALANCER
	if (on)
	{
		balancer_set_state(BALANCER_ACTIVE);
		strcpy_P(cmd_line, PSTR("Enabled balancer"));
	}
	else
	{
		balancer_set_state(BALANCER_SURVEILLANCE);
		strcpy_P(cmd_line, PSTR("Disabled balancer"));
	}
#endif
}

void cmd_welcome(void)
{
	strcpy_P(cmd_line, PSTR("Have a lot of fun..."));
}

void cmd_alpha(const char* cmd)
{
	ds1307_time_t time;
	
	time.CH = 0;
	time.format = 0;

	ds1307_encode_time(cmd, &time);

	ds1307_write_time(&time);

	ds1307_decode_time(&time, cmd_line);
}

void cmd_date(void)
{
	ds1307_time_t time;

	ds1307_read_time(&time);

	ds1307_decode_time(&time, cmd_line);
}

void cmd_binfo(void)
{
	strcpy_P(cmd_line, PSTR("\n\rBattery info:\n\r"));

	if (battery_info_get(REKUPERATION_NOK))
	{
		cmd_flush();
		strcpy_P(cmd_line, PSTR("Rekuperation nicht erlaubt\n\r"));
	}
	if (battery_info_get(CHARGE_NOK))
	{
		cmd_flush();
		strcpy_P(cmd_line, PSTR("Ladung nicht erlaubt\n\r"));
	}
	if (battery_info_get(DRIVE_NOK))
	{
		cmd_flush();
		strcpy_P(cmd_line, PSTR("Fahren nicht erlaubt\n\r"));
	}
	if (battery_info_get(CHARGE_CUR_TO_HI))
	{
		cmd_flush();
		strcpy_P(cmd_line, PSTR("Zu hoher Ladestrom\n\r"));
	}
	if (battery_info_get(DRIVE_CUR_TO_HI))
	{
		cmd_flush();
		strcpy_P(cmd_line, PSTR("Zu hoher Fahrstrom\n\r"));
	}
	if (battery_info_get(VOLTAGE_TO_HI))
	{
		cmd_flush();
		strcpy_P(cmd_line, PSTR("Zu hohe Ladespannung\n\r"));
	}
	if (battery_info_get(VOLTAGE_TO_LO))
	{
		cmd_flush();
		strcpy_P(cmd_line, PSTR("Zu tiefe Fahrspannung\n\r"));
	}
	if (battery_info_get(BAT_REL_OPEN))
	{
		cmd_flush();
		strcpy_P(cmd_line, PSTR("Batterie Relais offen\n\r"));
	}
	if (battery_info_get(BAT_FULL))
	{
		cmd_flush();
		strcpy_P(cmd_line, PSTR("Batterie voll\n\r"));
	}
	if (battery_info_get(BAT_EMPTY))
	{
		cmd_flush();
		strcpy_P(cmd_line, PSTR("Batterie entladen\n\r"));
	}
	if (battery_info_get(CHARGE_TEMP_TO_HI))
	{
		cmd_flush();
		strcpy_P(cmd_line, PSTR("Zu hohe Ladetemperatur\n\r"));
	}
	if (battery_info_get(CHARGE_TEMP_TO_LO))
	{
		cmd_flush();
		strcpy_P(cmd_line, PSTR("Zu tiefe Ladetemperatur\n\r"));
	}
	if (battery_info_get(DRIVE_TEMP_TO_HI))
	{
		cmd_flush();
		strcpy_P(cmd_line, PSTR("Zu hohe Fahrtemperatur\n\r"));
	}
	if (battery_info_get(DRIVE_TEMP_TO_LO))
	{
		cmd_flush();
		strcpy_P(cmd_line, PSTR("Zu tiefe Fahrtemperatur\n\r"));
	}
	if (battery_info_get(VOLTAGE_NOK))
	{
		cmd_flush();
		strcpy_P(cmd_line, PSTR("Unsymmetrische Spannungen\n\r"));
	}
	if (battery_info_get(BAT_ERROR))
	{
		cmd_flush();
		strcpy_P(cmd_line, PSTR("Fehler in der Batterieüberwachung\n\r"));
	}
}

void cmd_print_signed_float(int16_t value)
{
	if (value<0)
        {
                value = -value;
                strcat_P(cmd_line, PSTR("-"));
        }
	else
	{
		strcat_P(cmd_line, PSTR(" "));
	}

	cmd_print_unsigned_float(value);
}

void cmd_print_unsigned_float(uint16_t value)
{
	char temp[4];

	uint16_t high = value / 100;
	uint8_t low  = value % 100;

	strcat(cmd_line, itoa(high, temp, 10));
	strcat_P(cmd_line, PSTR("."));
	if (low>9)
	{
		strcat(cmd_line, itoa(low, temp, 10));
	}
	else
	{
		strcat_P(cmd_line, PSTR("0"));
		strcat(cmd_line, itoa(low, temp, 10));
	}

//	sprintf_P(cmd_line, PSTR("%d.%02d"), high, low);
}

void cmd_print_signed_fix(int16_t value)
{
	if (value<0)
        {
                value = -value;
                strcat_P(cmd_line, PSTR("-"));
        }
	else
	{
		strcat_P(cmd_line, PSTR(" "));
	}

	cmd_print_unsigned_fix(value);
}

void cmd_print_unsigned_fix(uint16_t value)
{
	char temp[6];

	strcat(cmd_line, itoa(value, temp, 10));
}

