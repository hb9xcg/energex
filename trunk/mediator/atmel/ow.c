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
 * @file        ow.c
 * @brief       OneWire-Treiber
 * @author      Markus Walser (markus.walser@gmail.com)
 * @date        18.11.2008
 */

#include "ow.h"
#include "i2c.h"
#include "os_thread.h"
#include <string.h>
#include <stdio.h>
#include "uart.h"

#define TRUE	1
#define FALSE	0

static uint8_t ds2482_address            = 0x30;
static uint8_t ds2482_reset              = 0xB4;
static uint8_t ds2482_busy               = 0xFE;
static uint8_t ds2482_pulse_detect       = 0xFC;
static uint8_t ds2482_device_reset       = 0x0F;
static uint8_t ds2482_bit_command        = 0x87;
static uint8_t ds2482_read_pointer_cmd   = 0xE1; // page 9
static uint8_t ds2482_status_register    = 0xF0; // page 9
static uint8_t ds2482_write_byte_cmd     = 0xA5;
static uint8_t ds2482_write_config_cmd   = 0xD2;
static uint8_t ds2482_read_byte_cmd      = 0x96;
static uint8_t ds2482_read_data_register = 0xE1;

static uint8_t search_serial_nbr[8];
static uint8_t last_discrepancy;


#ifdef DEBUG
    static uint8_t ow_debug[64];

    #define ERROR(x)        sprintf((char*)ow_debug, "%s:%d error=0x%x\r\n", __FUNCTION__, __LINE__, x); \
                            uart_write(ow_debug, strlen((char*)ow_debug));
#else
    #define ERROR(x)
#endif


int8_t ow_restart(void)
{
	i2c_write(ds2482_address, &ds2482_device_reset, 1 );
	uint8_t ret = i2c_wait();
	if (ret == TW_NO_INFO) {
		return eOWSuccess;
	} else {
		ERROR(ret);
		return eOWNoResponse;
	}
}

int8_t ow_wait_on_busy(void)
{
	uint8_t busy, ret;

	for (;;) {
		busy = 1;
		i2c_write_read(ds2482_address, NULL, 0, &busy, 1);
		ret = i2c_wait();
		if ( ret != TW_NO_INFO) {
			ERROR(ret);
			return eOWNoResponse;
		}
		busy |= ds2482_busy;
		if (busy == ds2482_busy) {
			break;	
		}
	}
	return eOWSuccess;
}

EOWError ow_reset(void)
{
	uint8_t test, ret;

	i2c_write(ds2482_address, &ds2482_reset, 1 );
	ret = i2c_wait();
	if ( ret != TW_NO_INFO) {
		ERROR(ret);
		return eOWNoResponse;
	}
	
	ow_wait_on_busy();
	
	i2c_write_read(ds2482_address, NULL, 0, &test, 1);
	ret = i2c_wait();
	if ( ret != TW_NO_INFO) {
		ERROR(ret);
		return eOWNoResponse;
	}
	test |= ds2482_pulse_detect;
	if (test == ds2482_busy) {
		return eOWSuccess;
	} else {
		return eOWNoPresencePulse;
	}	
}

int8_t ow_write_bit(uint8_t bit)
{
	uint8_t send_buf[2], ret;
	
	send_buf[0] = ds2482_bit_command;
	send_buf[1] = bit ? 0xFF : 0xFE;

	i2c_write(ds2482_address, send_buf, 2);
	ret = i2c_wait();
	if ( ret != TW_NO_INFO) {
		ERROR(ret);
		return eOWNoResponse;
	}
	
	return ow_wait_on_busy();	
}

int8_t ow_read_bit(void)
{
	uint8_t buffer[2], ret;
	
	ow_write_bit(1);
	
	buffer[0] = ds2482_read_pointer_cmd;
	buffer[1] = ds2482_status_register;

	i2c_write(ds2482_address, buffer, 2);
	ret = i2c_wait();
	if (ret != TW_NO_INFO) {
		ERROR(ret);
		return eOWNoResponse;
	}
	// TODO Prüfe, ob hier write und read in einem schnurz gemacht werden könnte!
	i2c_write_read(ds2482_address, NULL, 0, buffer, 1);
	ret = i2c_wait();
	if ( ret != TW_NO_INFO) {
		ERROR(ret);
		return eOWNoResponse;
	}

	if (buffer[0] & 0x20) {
		return 1;
	} else {
		return 0;
	}
}

EOWError ow_write_byte(uint8_t write_byte)
{
	uint8_t buffer[2];

	buffer[0] = ds2482_read_pointer_cmd;
	buffer[1] = ds2482_status_register;

	i2c_write(ds2482_address, buffer, 2);
	uint8_t ret = i2c_wait();
	if (ret != TW_NO_INFO) {
		ERROR(ret);
		return eOWNoResponse;
	}

	ow_wait_on_busy();

	buffer[0] = ds2482_write_byte_cmd;
	buffer[1] = write_byte;

	i2c_write(ds2482_address, buffer, 2);
	ret = i2c_wait();
	if (ret != TW_NO_INFO) {
		ERROR(ret);
		return eOWNoResponse;
	}

	return ow_wait_on_busy();
}

EOWError ow_read_byte(uint8_t* read_byte)
{
	uint8_t buffer[2], ret;

	buffer[0] = ds2482_read_pointer_cmd;
	buffer[1] = ds2482_status_register;

	i2c_write(ds2482_address, buffer, 2);
	ret = i2c_wait();
	if ( ret != TW_NO_INFO) {
		ERROR(ret);
		return eOWNoResponse;
	}

	ow_wait_on_busy();


	buffer[0] = ds2482_read_byte_cmd;

	i2c_write(ds2482_address, buffer, 1);
	ret = i2c_wait();
	if ( ret != TW_NO_INFO) {
		ERROR(ret);
		return eOWNoResponse;
	}

	
	buffer[0] = ds2482_read_pointer_cmd;
	buffer[1] = ds2482_read_data_register;

	i2c_write(ds2482_address, buffer, 2);
	ret = i2c_wait();
	if ( ret != TW_NO_INFO) {
		ERROR(ret);
		return eOWNoResponse;
	}
	// TODO Prüfe, ob hier write und read in einem schnurz gemacht werden könnte!
	i2c_write_read(ds2482_address, NULL, 0, read_byte, 1);
	ret = i2c_wait();
	if ( ret != TW_NO_INFO) {
		ERROR(ret);
		return eOWNoResponse;
	}

	return eOWSuccess;
}

void ow_block(void)
{
	
}

int8_t ow_triplet(uint8_t* direction, uint8_t* first_bit, uint8_t* second_bit)
{
	uint8_t send_buf[2], recv_buf;

	send_buf[0] = 0x78; // TODO why?
	
	if (*direction) {
		*direction = 0xFF;
	}
	send_buf[1] = *direction;

	// TODO Prüfe, ob hier write und read in einem schnurz gemacht werden darf!
	i2c_write(ds2482_address, send_buf, 2);
	uint8_t result = i2c_wait();
	if ( result != TW_NO_INFO) {
		// According to the application notes of the DS2482 something is fishy here.
		// For now we just ignore the I2C bus error, it seems to work anyway...
	}

	i2c_write_read(ds2482_address, NULL, 0, &recv_buf, 1);
	if (i2c_wait() != TW_NO_INFO) {
		return eOWNoResponse;
	}

	*first_bit  = ((recv_buf & 0x20) != 0);
	*second_bit = ((recv_buf & 0x40) != 0);
	*direction  = ((recv_buf & 0x80) != 0);
	
	return eOWSuccess;
}

int8_t ow_search(uint8_t reset_search, uint8_t* last_device, uint8_t* device_address)
{
	uint8_t result = eOWSuccess;
	uint8_t bit_number = 1;
	uint8_t last_zero  = 0;
	uint8_t serial_byte_number = 0;
	uint8_t serial_byte_mask = 1;
	uint8_t first_bit, second_bit, direction;
	uint8_t i = 0;

	if (reset_search) {
		last_device      = 0;
		last_discrepancy = 0;
	}

	if (!(*last_device)) {
		result = ow_reset();
		if ( result != eOWSuccess) {
			last_device = 0;
			last_discrepancy = 0;
			return result;
		}

		ow_write_byte(0xF0);	// Send "search ROM command"
	
		do {
			if (bit_number < last_discrepancy) {
				direction = ((search_serial_nbr[serial_byte_number] & serial_byte_mask) != 0);
			} else {
				direction = (bit_number == last_discrepancy);
			}
	
			result = ow_triplet( &direction, &first_bit, &second_bit);
			if ( result != eOWSuccess) {
				return result;
			}
	
			if (first_bit==0 && second_bit==0 && direction==0) {
				last_zero = bit_number;
			}
	
			if (first_bit==1 && second_bit==1) {
				break;
			}
	
			if (direction == 1) {
				search_serial_nbr[serial_byte_number] |=  serial_byte_mask;
			} else {
				search_serial_nbr[serial_byte_number] &= ~serial_byte_mask;
			}
	
			bit_number++;
			serial_byte_mask <<= 1;
			
			if (serial_byte_mask == 0) {
				serial_byte_number++;
				serial_byte_mask = 1;
			}
		} while (serial_byte_number < 8);
	
		result = eOWFailure;
		if (bit_number == 65) {
			last_discrepancy = last_zero;
			*last_device = last_discrepancy==0 ? TRUE : FALSE;
	
			for (i=0; i<8; i++) {
				device_address[i] = search_serial_nbr[i];
			}
	
			return eOWSuccess;
		}

		if (/*result==eOWFailure ||*/ !search_serial_nbr[0]) {
			last_discrepancy = 0;
			*last_device = 0;
		}
	}
	return result;
}

int8_t ow_level( uint8_t config_state, EOWLevel level)
{
	uint8_t buffer[2];
	
	buffer[0] = ds2482_write_config_cmd;
	if (level == eOWNormal) {
		buffer[1] = (config_state | 0x01) & 0xEF;
	} else {
		buffer[1] = (config_state | 0x10) & 0xFE;
	}
	
	i2c_write(ds2482_address, buffer, 2);
	if ( i2c_wait() != TW_NO_INFO) {
		return eOWNoResponse;
	} else {
		return eOWSuccess;
	}
}

void ow_read_bit_power(void)
{
}

EOWError ow_write_byte_power(uint8_t config_state, uint8_t write_byte)
{
	uint8_t buffer[2], ret;
	
	buffer[0] = ds2482_write_config_cmd;
	buffer[1] = (config_state | 0x04) & 0xBF;
	
	i2c_write(ds2482_address, buffer, 2);
	ret = i2c_wait();
	if ( ret != TW_NO_INFO) {
		ERROR(ret);
		return eOWNoResponse;
	}

	return ow_write_byte(write_byte);
}
