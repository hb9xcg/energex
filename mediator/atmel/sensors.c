/***************************************************************************
 *   Energex                                                               *
 *                                                                         *
 *   Copyright (C) 2008-2010 by Markus Walser                              *
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
 * @file        sensors.c
 * @brief       OneWire DS18S20 Temperatursensor-Treiber
 * @author      Markus Walser (markus.walser@gmail.com)
 * @date        23.11.2008
 */

#include <avr/io.h>
#include <string.h>
#include <stdio.h>
#include "sensors.h"
#include "ow.h"
#include "os_thread.h"
#include "delay.h"
#include "uart.h"
#include "crc8.h"
#include "io.h"

#define DS18S20_CMD_MATCH_ROM       0x55
#define DS18S20_CMD_CONVERT         0x44
#define DS18S20_CMD_READ_SCRATCH    0xBE
#define DS18S20_CRC_POLYNOM         0x31      // X8 + X5 + X4 + 1
#define DS18S20_SCRATCH_PAD_SIZE       9      // [bytes]
#define DS18S20_IDX_TEMPERATURE_LSB    0
#define DS18S20_IDX_TEMPERATURE_MSB    1
#define DS18S20_IDX_COUNT_REMAIN       6
#define DS18S20_IDX_COUNT_PER_GRAD     7
#define DS18S20_IDX_CRC                8

#define SENSORS_MAX_DEVICES        2
#define SENSORS_UNUSED             0x8000
#define SENSORS_USED               0x0000
#define SENSORS_STACK_SIZE         128




typedef struct
{
	uint8   serial[8];
	int16_t temp;
} sensor_t;


static uint8_t  sensors_stack[SENSORS_STACK_SIZE];
static sensor_t sensors[SENSORS_MAX_DEVICES];
static Tcb_t*   sensors_thread_tcb;

static void     sensors_thread(void);
static void     sensors_search(void);

#ifdef DEBUG
    static 	char line[20];
#endif

void sensors_init(void)
{
	int8_t i;
	for (i=0; i<SENSORS_MAX_DEVICES; i++) {
		sensors[i].temp = SENSORS_UNUSED;
	}

	sensors_search();

	memset(sensors_stack, 0xb6, sizeof(sensors_stack));
	sensors_thread_tcb = os_create_thread((uint8_t *)&sensors_stack[SENSORS_STACK_SIZE-1], sensors_thread );
}

uint16_t sensors_get_max_stack_usage(void)
{
	uint16_t stack_idx = 0;
	while(stack_idx < SENSORS_STACK_SIZE)
	{
		if (sensors_stack[stack_idx] != 0xb6)
		{
			break;
		}
		stack_idx++;
	}
	return SENSORS_STACK_SIZE-stack_idx;
}

int8_t sensors_get_nbr_of_devices(void)
{
	int8_t i, devices=0;
	
	os_enterCS();
	for (i=0; i<SENSORS_MAX_DEVICES; i++) {
	
		if (sensors[i].temp != SENSORS_UNUSED) {
			devices++;
		}
	}
	os_exitCS();
	return devices;
}

void sensors_get_temperatur(int8_t index, int16_t* temp)
{
	os_enterCS();
	if (index < SENSORS_MAX_DEVICES && 
            sensors[index].temp != SENSORS_UNUSED) {
		*temp = sensors[index].temp;
	} else {
		*temp = 0;
	}
	os_exitCS();
}

void sensors_get_min_temperatur(int16_t* temp)
{
	int8_t i;
	*temp = 9999;
	
	os_enterCS();
	for (i=0; i<SENSORS_MAX_DEVICES; i++) {
		if (sensors[i].temp != SENSORS_UNUSED) {
			if (sensors[i].temp < *temp) {
				*temp = sensors[i].temp;
			}
		}
	}
	os_exitCS();
}

void sensors_get_max_temperatur(int16_t* temp)
{
	int8_t i;
	*temp = -2999;
	
	os_enterCS();
	for (i=0; i<SENSORS_MAX_DEVICES; i++) {
		if (sensors[i].temp != SENSORS_UNUSED) {
			if (sensors[i].temp > *temp) {
				*temp = sensors[i].temp;
			}
		}
	}
	os_exitCS();
}

void sensors_get_avg_temperatur(int16_t* temp)
{
	int8_t i, nbr_of_devices=0;
	int32_t avg_temp=0;

	os_enterCS();
	for (i=0; i<SENSORS_MAX_DEVICES; i++) {
		if (sensors[i].temp != SENSORS_UNUSED) {
			avg_temp += sensors[i].temp;
			nbr_of_devices++;
		}
	}
	os_exitCS();

	if (nbr_of_devices) {
		avg_temp /= nbr_of_devices;
	}
	*temp = avg_temp;
}

void sensors_get_serial(int8_t index, uint8_t serial[])
{
	os_enterCS();
	if (sensors[index].temp != SENSORS_UNUSED) {
		memcpy(serial, sensors[index].serial, 8);
	}
	os_exitCS();
}

void sensors_thread(void)
{
	int8_t device=0;

	for (;;)
	{
		if (++device >= SENSORS_MAX_DEVICES) {
			device = 0;
		}
		if (sensors[device].temp == SENSORS_UNUSED) {
			continue;
		}
		sensors_start_conversion(sensors[device].serial);
		io_clear_green_led();
		delay(750);
		io_set_green_led();

		sensors_fetch_conversion(sensors[device].serial, &sensors[device].temp);
#ifdef DEBUG	
		sprintf( line, "Sensor[%d]: %d.%01d\r\n", 
			device, sensors[device].temp>>1, (sensors[device].temp&0x1)?5:0);
		uart_write((uint8_t*)line, strlen(line) );
#endif
	}
}

void sensors_search(void)
{	
	int8_t  idx=0, ret;
	uint8_t last_device=0;
	
	os_enterCS();

	for (idx=0; idx<SENSORS_MAX_DEVICES; idx++) {
		sensors[idx].temp = SENSORS_UNUSED;
	}

	idx = 0;
	while (idx < SENSORS_MAX_DEVICES)
	{
		io_clear_green_led();
		ret = ow_search( 0, &last_device, sensors[idx].serial);
		io_set_green_led();

		if( ret == eOWSuccess) {
			sensors[idx].temp = SENSORS_USED;
			idx++;
		} else {
			break;
		}
		
		if (last_device) {
			break;
			
		} else {
			delay(1280);
		}
	}
	os_exitCS();
}

EDSError sensors_start_conversion(uint8_t serial[])
{
	int8_t i;
	
	// Step 1: Initialisation
	EOWError ret = ow_reset();
	if (ret != eOWSuccess) {
		return ret;
	}

	// Step 2: ROM Command
	ret = ow_write_byte(DS18S20_CMD_MATCH_ROM);
	if (ret != eOWSuccess) {
		return ret;
	}
	for( i=0; i<8; i++) {
		ret = ow_write_byte(serial[i]);
		if (ret != eOWSuccess) {
			return ret;
		}
	}
	
	// Step 3: Function Command
	uint8_t config_state = 0xF0;
	return ow_write_byte_power(config_state, DS18S20_CMD_CONVERT);
}

void sensors_wait_conversion(void)
{
	os_thread_sleep(750);
}


EDSError sensors_fetch_conversion(uint8_t serial[], int16_t* temp)
{
	int8_t i;
	
	// Step 1: Initialisation
	EOWError ret = ow_reset();
	if (ret != eOWSuccess) {
		return ret;
	}

	// Step 2: ROM Command
	ow_write_byte(DS18S20_CMD_MATCH_ROM);
	for( i=0; i<8; i++) {
		ret = ow_write_byte(serial[i]);
		if (ret != eOWSuccess) {
			return ret;
		}
	}

	// Step 3: Function Command
	ret = ow_write_byte(DS18S20_CMD_READ_SCRATCH);
	if (ret != eOWSuccess) {
		return ret;
	}
	
	uint8_t value[DS18S20_SCRATCH_PAD_SIZE];
	for( i=0; i<DS18S20_SCRATCH_PAD_SIZE; i++) {
		ret = ow_read_byte( &value[i] );
		if (ret != eOWSuccess) {
			return ret;
		}
	}

	if (crc8(value, DS18S20_SCRATCH_PAD_SIZE-1) == value[DS18S20_IDX_CRC])
	{
		int16_t temperature = value[DS18S20_IDX_TEMPERATURE_LSB]; 
		
		if (value[DS18S20_IDX_TEMPERATURE_MSB])
		{
			temperature -= 0xFF;
		}
		temperature >>= 1;   // Truncate bit 0 for extended resolution
		temperature  *= 100; // create Twike temperature format.

		// extended resolution
		int16_t extended = -100;
		extended *= value[DS18S20_IDX_COUNT_REMAIN];
		extended /= value[DS18S20_IDX_COUNT_PER_GRAD];
		extended += 75;

		temperature += extended;

		os_enterCS();
		*temp = temperature;
		os_exitCS();
		
		ret = eDSSuccess;
	}
	else
	{
		ret = eDSCrcError;
	}

	return ret;
}

/* Scratchpad memory:
	Byte 0 Temperature LSB (AAh)
	Byte 1 Temperature MSB (00h) EEPROM
	Byte 2 TH Register or User Byte 1* TH Register or User Byte 1
	Byte 3 TL Register or User Byte 2* TL Register or User Byte 2
	Byte 4 Reserved (FFh)
	Byte 5 Reserved (FFh)
	Byte 6 COUNT REMAIN (0Ch)
	Byte 7 COUNT PER °C (10h)
	Byte 8 CRC*
*/

