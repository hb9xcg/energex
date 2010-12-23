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
#include "delay.h"
#include "idle.h"
#include "uart.h"
#include "os_thread.h"
#include "adc.h"
#include "battery.h"
#include "protocol.h"
#include "data.h"
#include "ko.h"
#include "plc.h"

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
static void cmd_line_voltage(void);
static void cmd_capacity(void);
static void cmd_reset(void);
static void cmd_reboot(void);
static void cmd_power(const char* cmd);
static void cmd_eeprom(const char* cmd);
static void cmd_stack_memory(void);
static void cmd_welcome(void);
static void cmd_flush(void);

static void cmd_print_signed_float(int16_t value);
static void cmd_print_unsigned_float(uint16_t value);
static void cmd_print_signed_fix(int16_t value);
static void cmd_print_unsigned_fix(uint16_t value);


void cmd_init(void)
{
	memset(cmd_stack, 0xb6, sizeof(cmd_stack));
	cmd_thread = os_create_thread((uint8_t *)&cmd_stack[COMMAND_STACK_SIZE-1], 
			cmd_dispatcher );	// Command thread anlegen
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

	delay(100);
	
	for(;;)
	{
		if (uart_data_available() > 0)
		{
			uart_read( &character, sizeof(character) );

			// Check debug escape sequence "^pp"
			if (lastCharacter==FRAME)
			{
				if (character=='p') 
				{
					eMode = eDebug;
					plc_force_busy(1);
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
			if (eMode == eTwike)
			{
				plc_busy();
				protocol_receive_byte(character);
			}
			else
			{
				if(character==FRAME)
				{
					eMode = eTwike;
					plc_force_busy(0);
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
			delay(8);
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
		strcpy_P(cmd_line, PSTR("\r\nroot@plc:~> "));
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
		case 'b':
			cmd_reboot();
		break;
		case 'e':
			cmd_eeprom(cmd);
		break;
		case 'i':
			cmd_current();
		break;
		case 'k':
			cmd_ko();
		break;
		case 'l':
			cmd_line_voltage();
		break;
		case 'm':
			cmd_stack_memory();
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
		case 't':
			cmd_temperatur();
		break;
		case 'u':
			cmd_voltage();
		break;
		case 'x':
			eMode = eTwike;
			plc_force_busy(0);
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
	strcpy_P(cmd_line, PSTR("i:\tCurrent\n\r"));
	cmd_flush();
	strcpy_P(cmd_line, PSTR("l:\tLine voltage\n\r"));
	cmd_flush();
	strcpy_P(cmd_line, PSTR("m\tMemory info\n\r"));
	cmd_flush();
	strcpy_P(cmd_line, PSTR("p:\tPower state {on|off}\n\r"));
	cmd_flush();
	strcpy_P(cmd_line, PSTR("q:\tCapacity\n\r"));
	cmd_flush();
	strcpy_P(cmd_line, PSTR("r:\tReset\n\r"));
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

void cmd_welcome(void)
{
	strcpy_P(cmd_line, PSTR("Have a lot of fun..."));
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
}

void cmd_reboot(void)
{
	data_save();
	wdt_enable(WDTO_15MS);
}

void cmd_temperatur(void)
{
	int16_t temperature;

	temperature = battery_get_parameter_value(TEMPERATUR1);

	strcpy_P( cmd_line, PSTR("Battery temperatur: "));
	cmd_print_signed_float(temperature);
	strcpy_P( cmd_line, PSTR("°C"));
}


void cmd_voltage(void)
{
	uint16_t voltage;

	voltage = battery_get_voltage();

	strcpy_P( cmd_line, PSTR("Battery voltage: "));
	cmd_print_unsigned_float(voltage);	
	strcat_P( cmd_line, PSTR("V"));
}

void cmd_current(void)
{
	int16_t current;

	current = battery_get_current();

	strcpy_P( cmd_line, PSTR("Battery current: "));
	cmd_print_signed_float(current);
	strcat_P( cmd_line, PSTR("A"));
}

void cmd_capacity(void)
{
	int16_t capacity;
	
	capacity  = battery_get_parameter_value(AH_ZAEHLER) * 3;

	strcpy_P( cmd_line, PSTR("Battery actual capacity: "));
	cmd_print_signed_float(capacity);
	strcat_P( cmd_line, PSTR("Ah\n\r"));

	cmd_line[0] = '\0';
}

void cmd_reset(void)
{
	strcpy_P( cmd_line, PSTR("Mediator resetted capacity to 0mAh"));
}

void cmd_power(const char* cmd)
{
	if( strstr( cmd, "off") )
	{
		io_open_relais();
		strcpy_P(cmd_line, PSTR("Switched to power state \'off\'"));
	}
	else if( strstr( cmd, "on") )
	{
		io_close_relais();
		strcpy_P(cmd_line, PSTR("Switched to power state \'on\'"));
	}
	else
	{
		strcpy_P(cmd_line, PSTR("Use \'on\' or \'off\' as argument"));
	}
}

void cmd_stack_memory(void)
{
	uint16_t result;
		
	result = cmd_get_max_stack_usage();	
	strcpy_P(cmd_line, PSTR("command_thread's max stack usage: "));
	cmd_print_unsigned_fix(result);
	strcat_P(cmd_line, PSTR(" bytes\r\n"));

	cmd_flush();

	result = idle_get_max_stack_usage();	
	strcpy_P(cmd_line, PSTR("idle_thread's max stack usage: "));
	cmd_print_unsigned_fix(result);
	strcat_P(cmd_line, PSTR(" bytes\r\n"));
}

void cmd_ko(void)
{
	ko_print();
}

void cmd_line_voltage(void)
{
	uint16_t voltage = plc_get_line_voltage();
	strcpy_P(cmd_line, PSTR("Actual line voltage: "));
	cmd_print_unsigned_fix(voltage);
	strcat_P(cmd_line, PSTR("V"));
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

	uint8_t high = value / 100;
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
	char temp[7];

	strcat(cmd_line, itoa(value, temp, 10));
}

