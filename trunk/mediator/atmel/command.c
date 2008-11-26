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
 * @file 	command.c
 * @brief 	User debug prompt.
 * @author 	Markus Walser (markus.walser@gmail.com)
 * @date 	11.02.08
 */
#include <avr/io.h>
#include <string.h>
#include <stdio.h>

#include "command.h"
#include "os_thread.h"
#include "uart.h"
#include "adc.h"
#include "charge.h"
#include "battery.h"
#include "simulator.h"
#include "protocol.h"
#include "ow.h"
#include "sensors.h"

#define COMMAND_STACK_SIZE 256
#define CMD_LINE   256
#define DEBUG(str)  uart_write((uint8_t*)(str), sizeof(str) );

#define FRAME 		0x10

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
static void cmd_capacity(void);
static void cmd_reset(void);
static void cmd_offset(void);
static void cmd_power(const char* cmd);
static void cmd_onewire(const char* cmd);
static void cmd_sensor(const char* cmd);
static void cmd_comm_simulator(const char* cmd);


void cmd_init(void)
{
	cmd_thread = os_create_thread((uint8_t *)&cmd_stack[COMMAND_STACK_SIZE-1], cmd_dispatcher );	// Command thread anlegen
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
					character = '?';
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
			if (eMode == eTwike)
			{
				protocol_receive_byte(character);
			}
			else
			{
				cmd_receive_byte(character);
			}
		}
		else
		{
			os_thread_sleep(50);
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
		DEBUG("\r\n> ");
		lineIdx=0;
		cmd_line[lineIdx] = '\0';
	}
	else if (character == '\0')
	{
		PORTC ^= LED_RED;
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
		case 'c':
			cmd_comm_simulator(cmd);
		break;
		case 'i':
			cmd_current();
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
			cmd_sensor(cmd);
		break;
		case 't':
			cmd_temperatur();
		break;
		case 'u':
			cmd_voltage();
		break;
		case 'w':
			cmd_onewire(cmd);
		break;
		case 'x':
			eMode = eTwike;
		break;
		case '?':
			cmd_help();
		break;
		default:
			strcpy(cmd_line, "<Unknown command, try ? for help.");
	}
	uart_write( (uint8_t*)cmd_line, strlen(cmd_line) );
}

void cmd_help(void)
{
	strcpy(cmd_line, "\n\rAvailable commands:\n\r");
	strcat(cmd_line, "i:\tCurrent\n\r");
	strcat(cmd_line, "p:\tPower state {full|save|off}\n\r");
	strcat(cmd_line, "q:\tCapacity\n\r");
	strcat(cmd_line, "r:\tReset\n\r");
	strcat(cmd_line, "s:\tSensor\n\r");
	strcat(cmd_line, "t:\tTemperatur\n\r");
	strcat(cmd_line, "u:\tVoltage\n\r");
//	strcat(cmd_line, "c:\tCommunication simulation {start|stop}\n\r");
	strcat(cmd_line, "x:\tExit\n\r");
	strcat(cmd_line, "?:\tHelp\n\r");
}

void cmd_comm_simulator(const char* cmd)
{
	if( strstr( cmd, "start") )
	{
		simulator_activate();
		strcpy(cmd_line, "Activated communication simulator.");
	}
	else if( strstr( cmd, "stop") )
	{
		simulator_deactivate();
		strcpy(cmd_line, "Deactivated communication simulator.");
	}
}

void cmd_temperatur(void)
{
	int16_t temperature;

	temperature = getTemperature();

	sprintf( cmd_line, "Mediator temperatur: %d.%02d°C", temperature/100, temperature%100);
}

void cmd_voltage(void)
{
	int16_t voltage;

	voltage = getVoltage();

	sprintf( cmd_line, "Mediator voltage: %d.%02dV", voltage/100, voltage%100);
}

void cmd_current(void)
{
	int16_t current;
	int8_t sign = ' ';
	
	current = charge_get_current();

	if (current<0) {
		sign = '-';
		current = -current;
	}

	sprintf( cmd_line, "Mediator current: %c%d.%02dA", sign, current/100, current%100);
}

void cmd_offset(void)
{
	int16_t offset;
	
	offset = adc_get_offset();
	
	sprintf( cmd_line, "Mediator ADC offset: %d", offset);

	adc_calibrate_offset();
}

void cmd_capacity(void)
{
	int16_t capacity;
	int8_t sign = ' ';

   	capacity = charge_get_capacity();

	if (capacity<0) {
		sign = '-';
		capacity = -capacity;
	}

	sprintf( cmd_line, "Mediator capacity: %c%d.%02dAh", sign, capacity/100, capacity%100);
}

void cmd_reset(void)
{
	charge_reset();

	sprintf( cmd_line, "Mediator resetted capacity to 0mAh");
}

static void cmd_sensor(const char* cmd)
{
	int8_t nbr_of_sensors;
	uint8_t idx, serial[8];
	int16_t temp;

	char* header = "device serial number      temperatur\n\r";
	uart_write( (uint8_t*)header, strlen(header) );
	
	nbr_of_sensors = sensors_get_nbr_of_devices();
	for(idx=0; idx<nbr_of_sensors; idx++)
	{
		temp = 0;
		sensors_get_temperatur( idx, &temp);
		sensors_get_serial( idx, serial);
		sprintf( cmd_line, "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x       %d.%01d°C\r\n",
				serial[0], 
				serial[1], 
				serial[2],
				serial[3],
				serial[4],
				serial[5],
				serial[6],
				serial[7],
				temp>>1, 
				5*(temp&0x1));

		uart_write( (uint8_t*)cmd_line, strlen(cmd_line) );
		uart_flush();
	}
	strcpy(cmd_line, "====================================");
}

void cmd_power(const char* cmd)
{
	if( strstr( cmd, "off") )
	{
		ePowerSoll = ePowerOff;
		strcpy(cmd_line, "Switched to power state \'off\'");
	}
	else if( strstr( cmd, "full") )
	{
		ePowerSoll = ePowerFull;
		strcpy(cmd_line, "Switched to power state \'full\'");
	}
	else if( strstr( cmd, "save") )
	{
		ePowerSoll = ePowerSave;
		strcpy(cmd_line, "Switched to power state \'save\'");
	}
	else
	{
		strcpy(cmd_line, "Use \'full\', \'save\' or \'off\' as argument");
	}
}


void cmd_onewire(const char* cmd)
{
	int8_t result;
	if( strstr( cmd, "restart") )
	{
		result = ow_restart();
		sprintf( cmd_line, "ow_restart returned %d", result);
	}
	else if (strstr( cmd, "reset") )
	{
		result = ow_reset();
		sprintf( cmd_line, "ow_reset returned %d", result);
	}
	else if (strstr( cmd, "search"))
	{
		ow_reset();

		uint8_t last_device = 0;
		uint8_t snum[8];
		uint8_t nbr_of_dev = 0;

		for (;;)
		{
			PORTC |= LED_RED;

			result = ow_search( 0, &last_device, snum);
			sprintf( cmd_line, "one wire dev: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\r\n",
				snum[0], snum[1], snum[2], snum[3], snum[4], snum[5], snum[6], snum[7]);
			uart_write( (uint8_t*)cmd_line, strlen(cmd_line) );
			
			PORTC &= ~LED_RED;

			if( result == 0) {
				nbr_of_dev++;
			}
			
			if (!last_device) {
				os_thread_sleep(1280);
			} else {
				break;
			}
		}
		
		sprintf( cmd_line, "ow_search found %d one wire device%s", nbr_of_dev, nbr_of_dev>1 ? "s" : "");
	}
	else
	{
		strcpy(cmd_line, "Use \'restart\', \'reset\' or \'search\' as argument");
	}
}
