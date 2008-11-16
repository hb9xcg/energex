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
#include "supervisor.h"
#include "simulator.h"
#include "protocol.h"

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
static void cmd_supervisor(const char* cmd);
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
		os_thread_sleep(100);
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
			cmd_supervisor(cmd);
		break;
		case 't':
			cmd_temperatur();
		break;
		case 'u':
			cmd_voltage();
		break;
		case '?':
			cmd_help();
		break;
		case 'x':
			eMode = eTwike;
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
	strcat(cmd_line, "s:\tSupervisor {on|off|info}\n\r");
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

static void cmd_supervisor(const char* cmd)
{
	if( strstr( cmd, "on") )
	{
		supervisor_activate();
		strcpy(cmd_line, "Activated supervisor bus.");
	}
	else if( strstr( cmd, "off") )
	{
		supervisor_deactivate();
		strcpy(cmd_line, "Deactivated supervisor bus.");
	}
	else if( strstr( cmd, "info") )
	{
		cell_t info;
		int8_t nbr_of_info;
		uint8_t idx;

		char* header = "cell temp voltage err\n\r";
		uart_write( (uint8_t*)header, strlen(header) );
		
		char* line   = (char*)&cmd_line[0]; // temporarily available memory
		nbr_of_info = supervisor_get_nbr_of_info();
		for(idx=0; idx<nbr_of_info; idx++)
		{
			supervisor_get_info( idx, &info);
			sprintf( line, "%4d %2d°C %5dmV %#x\n\r", idx, info.temperture, info.voltage, info.error);
			uart_write( (uint8_t*)line, strlen(line) );
			uart_flush();
		}
		strcpy(cmd_line, "=====================");
	}
	else
	{
		strcpy(cmd_line, "Use \'on\', \'off\' or \'info\' as argument");
	}
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

