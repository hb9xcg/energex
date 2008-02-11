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
#include "supervisor.h"

#define COMMAND_STACK_SIZE 256
#define CMD_LINE   128
#define DEBUG(str)  uart_write((uint8_t*)(str), sizeof(str) );

uint8_t cmd_stack[COMMAND_STACK_SIZE];
static Tcb_t * cmd_thread;
static char cmd_answer[CMD_LINE];
static uint8_t cmd_line[CMD_LINE];

static void cmd_dispatcher(void);
static void cmd_process( const char* cmd);
static void cmd_help(void);
static void cmd_temperatur(void);
static void cmd_voltage(void);
static void cmd_current(void);
static void cmd_capacity(void);
static void cmd_reset(void);
static void cmd_power(const char* cmd);
static void cmd_supervisor(const char* cmd);

void cmd_init(void)
{
	cmd_thread = os_create_thread((uint8_t *)&cmd_stack[COMMAND_STACK_SIZE-1], cmd_dispatcher );	// Command thread anlegen
}


void cmd_dispatcher(void)
{
	uint8_t lineIdx=0;
	uint8_t character;

	os_thread_sleep(100);
	
	for(;;)
	{
		if( uart_data_available() > 0)
		{
			uart_read( &character, 1 );
			uart_write( &character, 1 );

			switch( character )
			{
			case 0x8:
				lineIdx--;
				break;
//			case '\n':
			case '\r':
			{
				// Line completed
				DEBUG("\r\n");
				cmd_line[lineIdx] = '\0';
				cmd_process((char*)cmd_line);
				DEBUG("\r\n>");
				lineIdx=0;
				cmd_line[lineIdx] = '\0';
			}
			break;
			default:
				cmd_line[lineIdx] = character;
				lineIdx++;	
			}
		}
		os_thread_sleep(50);
	}
}

void cmd_process( const char* cmd )
{
	switch( cmd[0] )
	{
		case 'i':
			cmd_current();
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
		default:
			strcpy(cmd_answer, "Unknown command, try ? for help.");
	}
	uart_write( (uint8_t*)cmd_answer, strlen(cmd_answer) );
}

void cmd_help(void)
{
	strcpy(cmd_answer, "\n\rAvailable commands:\n\r");
	strcat(cmd_answer, "i:\tCurrent\n\r");
	strcat(cmd_answer, "p:\tPower state {full|save|off}\n\r");
	strcat(cmd_answer, "q:\tCapacity\n\r");
	strcat(cmd_answer, "r:\tReset\n\r");
	strcat(cmd_answer, "s:\tSupervisor {on|off|info}\n\r");
	strcat(cmd_answer, "t:\tTemperatur\n\r");
	strcat(cmd_answer, "u:\tVoltage\n\r");
	strcat(cmd_answer, "?:\tHelp\n\r");
}

void cmd_temperatur(void)
{
	int16_t value = 0;
	int32_t temperatur;

	adc_read_int(4, &value);

	os_thread_sleep(20);

	temperatur = value;
	temperatur *= 2500;// Reference [mV]
	temperatur /= 1024;
//	temperatur -= 600;

	sprintf( cmd_answer, "IController temp voltage: %dmV", (int16_t)temperatur);
//	sprintf( cmd_answer, "IController temperatur: %d°C", (int16_t)temperatur/10);
}

void cmd_voltage(void)
{
	int16_t value = 0;
	int32_t voltage;

	adc_read_int(5, &value);

	os_thread_sleep(20);

	voltage = value;
	voltage *= 2500;// Reference [mV]
	voltage *= 146; // calculate voltage divider
	voltage /= 1024;

//	sprintf( cmd_answer, "IController voltage: %dmV", (int16_t)voltage);
	sprintf( cmd_answer, "IController voltage: %d.%dV", (int16_t)((voltage+500)/1000), (int16_t)((voltage%1000+50)/100));
}

void cmd_current(void)
{
	int16_t current;
	
	charge_get_current( &current );
	
	sprintf( cmd_answer, "IController current: %dmA", current);
}

void cmd_capacity(void)
{
	int16_t capacity;

    charge_get_capacity( &capacity );

	sprintf( cmd_answer, "IController capacity: %dmAh", capacity);
}

void cmd_reset(void)
{
	charge_reset();

	sprintf( cmd_answer, "IController resetted capacity to 0mAh");
}

static void cmd_supervisor(const char* cmd)
{
	cell_t info;

	if( strstr( cmd, "on") )
	{
		supervisor_activate();
		strcpy(cmd_answer, "Activated supervisor bus.");
	}
	else if( strstr( cmd, "off") )
	{
		supervisor_deactivate();
		strcpy(cmd_answer, "Deactivated supervisor bus.");
	}
	else if( strstr( cmd, "info") )
	{
		int8_t nbr_of_info;
		uint8_t idx;		
		char line[22];

		char* header = "cell temp voltage err\n\r";
		uart_write( (uint8_t*)header, strlen(header) );

		nbr_of_info = supervisor_get_nbr_of_info();
		for(idx=0; idx<nbr_of_info; idx++)
		{
			supervisor_get_info( idx, &info);
			sprintf( line, "%4d %2d°C %5dmV %3x\n\r", idx, info.temperture, info.voltage, info.error);
			uart_write( (uint8_t*)line, strlen(line) );
			uart_flush();
		}
	}
}

void cmd_power(const char* cmd)
{
	if( strstr( cmd, "off") )
	{
		ePowerSoll = ePowerOff;
		strcpy(cmd_answer, "Switched to power state \'off\'");
	}
	else if( strstr( cmd, "full") )
	{
		ePowerSoll = ePowerFull;
		strcpy(cmd_answer, "Switched to power state \'full\'");
	}
	else if( strstr( cmd, "save") )
	{
		ePowerSoll = ePowerSave;
		strcpy(cmd_answer, "Switched to power state \'save\'");
	}
	else
	{
		strcpy(cmd_answer, "Use \'full\', \'save\' or \'off\' as argument");
	}
}

