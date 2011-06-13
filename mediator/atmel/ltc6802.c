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
 * @file        ltc6802.c
 * @brief       Battery Stack Monitor driver
 * @author      Markus Walser (markus.walser@gmail.com)
 * @date        14.11.2009
 */

#include "ltc6802.h"
#include <avr/pgmspace.h>
#include "os_thread.h"
#include "delay.h"
#include "spi.h"
#include "ntc.h"
#include "error.h"
#include "io.h"
#include "data.h"

#define TRUE	1
#define FALSE	0

#define WRCFG	0x01	// Write configuration register group.
#define RDCFG	0x02	// Read configuration register group.
#define RDCV	0x04	// Read cell voltage register group.
#define RDFLG	0x06	// Read flag register group.
#define RDTMP	0x08	// Read temperature register group.
#define STCVAD	0x10	// Start cell voltage A/D conversions and poll status .
#define STOWAD	0x20	// Start open wire A/D conversions and poll status.
#define STTMPAD	0x30	// Start temperature A/D conversions and poll status.
#define PLADC	0x40	// Poll A/D converter status.
#define PLINT	0x50	// Poll interrupt status.
#define STCVDC	0x60	// Start cell voltage conversions and poll status, 
			// with discharge permitted.
#define STOWDC	0x70	// Start open wire A/D conversions and poll status,
			// with discharge permitted.

#define ADDRESS 0x80    // Base address for addressed commands.

#define LTC_CS	(1<<PB4)	// 0x10 // Chip select
#define LTC_SDI	(1<<PB6)	// 0x05	// LTC6802 -> ATMega
#define LTC_SDO	(1<<PB5) 	// 0x06 // ATMega  -> LTC6802 
#define LTC_CLK	(1<<PB7) 	// 0x07 // Clock

#define DEBUG(str)  uart_write((uint8_t*)(str), sizeof(str) );
#if 0 
    #include <string.h>
    #include <stdlib.h> 
    #include "uart.h"
    static char ltc_debug[16];
    static char ltc_temp[16];

    #define TRACE(x)        strcpy(ltc_debug, itoa(x, ltc_temp, 10)); \
                            strcat(ltc_debug, "\r\n");                \
                            uart_write((uint8_t*)ltc_debug, strlen((char*)ltc_debug));
#else
    #define TRACE(x)
#endif

prog_uint8_t ltc_table[] = 
{
    0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15, 0x38, 0x3F, 0x36, 0x31,
    0x24, 0x23, 0x2A, 0x2D, 0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65,
    0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D, 0xE0, 0xE7, 0xEE, 0xE9,
    0xFC, 0xFB, 0xF2, 0xF5, 0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
    0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85, 0xA8, 0xAF, 0xA6, 0xA1,
    0xB4, 0xB3, 0xBA, 0xBD, 0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2,
    0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA, 0xB7, 0xB0, 0xB9, 0xBE,
    0xAB, 0xAC, 0xA5, 0xA2, 0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
    0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32, 0x1F, 0x18, 0x11, 0x16,
    0x03, 0x04, 0x0D, 0x0A, 0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42,
    0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A, 0x89, 0x8E, 0x87, 0x80,
    0x95, 0x92, 0x9B, 0x9C, 0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
    0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC, 0xC1, 0xC6, 0xCF, 0xC8,
    0xDD, 0xDA, 0xD3, 0xD4, 0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C,
    0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44, 0x19, 0x1E, 0x17, 0x10,
    0x05, 0x02, 0x0B, 0x0C, 0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
    0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B, 0x76, 0x71, 0x78, 0x7F,
    0x6A, 0x6D, 0x64, 0x63, 0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B,
    0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13, 0xAE, 0xA9, 0xA0, 0xA7,
    0xB2, 0xB5, 0xBC, 0xBB, 0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
    0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB, 0xE6, 0xE1, 0xE8, 0xEF,
    0xFA, 0xFD, 0xF4, 0xF3
};

static ltc_crv_t    ltc_voltages[LTC_STACK_SIZE];
static ltc_tmpr_t   ltc_temperatures[LTC_STACK_SIZE];
static ltc_config_t ltc_config[LTC_STACK_SIZE];

static uint8_t ltc_crc(uint8_t const buffer[], const uint8_t size);
static void ltc_chip_select(uint8 state);
static uint16_t ltc_get_voltage_internal(const ltc_crv_t* voltage, uint8_t idx);
static void ltc_calc_ber(void);

static uint16_t ltc_bytes_ok;
static uint16_t ltc_bytes_bad;


void ltc_init(void)
{
	DDRB  |=  LTC_CS;  // Output
	PORTB |=  LTC_CS;  // CS disabled

	ltc_config_t config;

	config.cdc    =   4; // 500ms OV/UV period, 13ms cell measurement
	config.cell10 =   0; // 12 cell mode
	config.lvlpl  =   0; // toggle polling
	config.gpio1  =   1; // pulldown disabled
	config.gpio2  =   1; // pulldown disabled
	config.wdten  =   1; // Watchdog enabled
	config.dcc    =   0; // All discharge switches off
	config.mci    =   0; // All cell interrupts enabled
	config.vuv    = 146; // Under voltage comparison 3.5V
//	config.vov    = 167; // Over voltage comparision 4.0V
	config.vov    = 173; // Over voltage comparision 4.15V
	
	for (uint8_t block=0; block<LTC_STACK_SIZE; block++)
	{
		ltc_config[block] = config;
	}

	ltc_write_config();
}

void ltc_set_standby_mode()
{
	ltc_config_t config;

	config.cdc    = 0; // Standby mode
	config.cell10 = 0; // 12 cell mode
	config.lvlpl  = 0; // toggle polling
	config.gpio1  = 1; // pulldown disabled
	config.gpio2  = 1; // pulldown disabled
	config.wdten  = 1; // Watchdog enabled
	config.dcc    = 0; // All discharge switches off
	config.mci    = 0; // All cell interrupts enabled
	config.vuv    = 0; // Under voltage comparison disabled for all cells.
	config.vov    = 0; // Over voltage comparision disabled for all cells.
	
	for (uint8_t block=0; block<LTC_STACK_SIZE; block++)
	{
		ltc_config[block] = config;
	}

	ltc_write_config();
}

void ltc_chip_select(uint8 state)
{
	if (state)
	{
		PORTB &= (uint8_t)~LTC_CS;  // CS enabled (low)
	}
	else
	{
		PORTB |= (uint8_t)LTC_CS;  // CS disabled (high)
	}
}

void ltc_write_config()
{
	int8_t block;
	uint8_t address = ADDRESS;

	for (block=0; block<LTC_STACK_SIZE; block++)
	{
		ltc_chip_select(TRUE);
		spi_master_transmit(address++);
		spi_master_transmit(WRCFG);

		ltc_config_t* config = &ltc_config[block];
		for (uint8 i=0; i<sizeof(ltc_config_t); i++)
		{
			spi_master_transmit(config->cfgr[i]);
		}
		ltc_chip_select(FALSE);
		_delay_loop_2(16); // 4us
	}

}

void ltc_start_voltage_conversion(void)
{
	ltc_chip_select(TRUE);
	spi_master_transmit(STCVAD);
	ltc_chip_select(FALSE);
}

void ltc_read_voltage(void)
{
	ltc_crv_t new_value;
	uint8_t address = ADDRESS;

	io_clear_red_led();

	for (uint8_t block=0; block<LTC_STACK_SIZE; block++)
	{
		ltc_chip_select(TRUE);
		spi_master_transmit(address++);
		spi_master_transmit(RDCV);

		ltc_bytes_ok += sizeof(ltc_crv_t);
		for (uint8_t idx=0; idx<sizeof(ltc_crv_t); idx++)
		{
			new_value.crv[idx] = spi_master_receive();
		}
		uint8_t pec = spi_master_receive();
		uint8_t crc = ltc_crc(new_value.crv, sizeof(ltc_crv_t));
		if (pec == crc)
		{
			os_enterCS();
			ltc_voltages[block] = new_value;
			os_exitCS();
		}
		else
		{
			ltc_bytes_bad += sizeof(ltc_crv_t);
			io_set_red_led();
		}
		ltc_chip_select(FALSE);
		_delay_loop_2(16); // 4us
	}
	ltc_calc_ber();

}

void ltc_start_temperature_conversion(void)
{
	ltc_chip_select(TRUE);
	spi_master_transmit(STTMPAD);
	ltc_chip_select(FALSE);
}

void ltc_read_temperature()
{
	ltc_tmpr_t new_value;
	uint8_t address = ADDRESS;
	
	io_clear_red_led();

	for (uint8_t block=0; block<LTC_STACK_SIZE; block++)
	{
		ltc_chip_select(TRUE);

		spi_master_transmit(address++);
		spi_master_transmit(RDTMP);

		ltc_bytes_ok += sizeof(ltc_tmpr_t);
		for (uint8_t idx=0; idx<sizeof(ltc_tmpr_t); idx++)
		{
			new_value.tmpr[idx] = spi_master_receive();
		}
		uint8_t pec = spi_master_receive();
		uint8_t crc = ltc_crc(new_value.tmpr, sizeof(ltc_tmpr_t));
		if (pec == crc)
		{
			os_enterCS();
			ltc_temperatures[block] = new_value;
			os_exitCS();
		}
		else
		{
			ltc_bytes_bad += sizeof(ltc_tmpr_t);
			io_set_red_led();
		}
		ltc_chip_select(FALSE);
		_delay_loop_2(16); // 4us
	}
	ltc_calc_ber();
}

int16_t ltc_get_internal_temperature(uint8_t index)
{

	os_enterCS();
	int32_t temp = ltc_temperatures[index].itmp;
	os_exitCS();
	temp *= 150;
	temp >>= 3;
	temp -= 27315;

	return temp;
}

int16_t ltc_get_external_temperature(uint8_t index)
{

	int16_t temp;
       
	os_enterCS();
	if (index & 0x1)
	{
		temp = ltc_temperatures[index>>1].etmp2;
	}
	else
	{
		temp = ltc_temperatures[index>>1].etmp1;
	}
	os_exitCS();

	return ntc_get_temp(temp);
}

void ltc_get_temperature_min_avg_max(int16_t *out_min, int16_t *out_avg, int16_t *out_max)
{
	uint16_t min=65535, max=0, avg=0, temperatur;

	os_enterCS();
	
	for (int8_t idx=0; idx<LTC_STACK_SIZE; idx++)
	{
		const ltc_tmpr_t* chip = &ltc_temperatures[idx];

		temperatur = chip->etmp1;
		if (temperatur < min)
		{
			min = temperatur;
		}
		if (temperatur > max)
		{
			max = temperatur;
		}
		avg += temperatur;
		
		temperatur = chip->etmp2;
		if (temperatur < min)
		{
			min = temperatur;
		}
		if (temperatur > max)
		{
			max = temperatur;
		}
		avg += temperatur;
	}

	os_exitCS();

	avg >>= 4;

	*out_min = ntc_get_temp(max);
	*out_avg = ntc_get_temp(avg);
	*out_max = ntc_get_temp(min);
}

int16_t ltc_get_temperature_min(void)
{
	uint16_t min=65535;

	os_enterCS();
	
	for (int8_t idx=0; idx<LTC_STACK_SIZE; idx++)
	{
		const ltc_tmpr_t* chip = &ltc_temperatures[idx];

		if (chip->etmp1 < min)
		{
			min = chip->etmp1 ;
		}
		
		if (chip->etmp2 < min)
		{
			min = chip->etmp2;
		}
	}

	os_exitCS();

	return ntc_get_temp(min);
}

int16_t ltc_get_temperature_max(void)
{
	uint16_t max=0;

	os_enterCS();
	
	for (int8_t idx=0; idx<LTC_STACK_SIZE; idx++)
	{
		const ltc_tmpr_t* chip = &ltc_temperatures[idx];

		if (chip->etmp1 > max)
		{
			max = chip->etmp1 ;
		}
		
		if (chip->etmp2 > max)
		{
			max = chip->etmp2;
		}
	}

	os_exitCS();

	return ntc_get_temp(max);
}
uint8_t ltc_crc(uint8_t const buffer[], const uint8_t size)
{
    uint8_t crc = 0;
    uint8_t idx = 0;

    for (idx=0; idx<size; idx++)
    {
	crc = buffer[idx] ^ crc;
	crc = pgm_read_byte_near(ltc_table+crc);
    }

    return crc;
}

uint16_t ltc_get_voltage(const uint8_t cell)
{
	int8_t block=0, j;

	for (j=12, block=0; j<96; j+=12, block++)
	{
		if (cell<j)
		{
			break;
		}
	}

	return ltc_get_voltage_internal( &ltc_voltages[block], cell+12-j);
}

uint16_t ltc_get_voltage_internal(const ltc_crv_t* voltage, uint8_t cell)
{
	uint16_t u;
	
	switch(cell)
	{
	case  0:	u = voltage->c1v;	break;
	case  1:	u = voltage->c2v;	break;
	case  2:	u = voltage->c3v;	break;
	case  3:	u = voltage->c4v;	break;
	case  4:	u = voltage->c5v;	break;
	case  5:	u = voltage->c6v;	break;
	case  6:	u = voltage->c7v;	break;
	case  7:	u = voltage->c8v;	break;
	case  8:	u = voltage->c9v;	break;
	case  9:	u = voltage->c10v;	break;
	case 10:	u = voltage->c11v;	break;
	case 11:	u = voltage->c12v;	break;
	default:	u = 0;
	}

	return u;
}

void ltc_get_voltage_limits(uint16_t* out_min, uint16_t* out_max)
{
	uint16_t min=32768, max=0;

	os_enterCS();
	
	for (int8_t idx=0; idx<LTC_STACK_SIZE; idx++)
	{
		const ltc_crv_t* chip = &ltc_voltages[idx];
		for (int8_t cell=0; cell<12; cell++)
		{
			uint16_t voltage = ltc_get_voltage_internal(chip, cell);
			if (voltage < min)
			{
				min = voltage;
			}
			if (voltage > max)
			{
				max = voltage;
			}
		}
	}

	os_exitCS();

	*out_min = min;
	*out_max = max;
}

void ltc_get_voltage_min_avg_max(int16_t* out_min, int16_t* out_avg, int16_t* out_max)
{
	uint16_t min=32768, max=0;
	uint32_t avg=0;

	os_enterCS();
	
	for (int8_t idx=0; idx<LTC_STACK_SIZE; idx++)
	{
		const ltc_crv_t* chip = &ltc_voltages[idx];
		for (int8_t cell=0; cell<12; cell++)
		{
			uint16_t voltage = ltc_get_voltage_internal(chip, cell);
			if (voltage < min)
			{
				min = voltage;
			}
			if (voltage > max)
			{
				max = voltage;
			}
			avg += voltage;
		}
	}

	os_exitCS();

	avg /= (LTC_STACK_SIZE*12);

	*out_min = min;
	*out_avg = avg;
	*out_max = max;
}

uint16_t ltc_get_max_voltage(void)
{
	uint16_t max=0;

	os_enterCS();
	
	for (int8_t idx=0; idx<LTC_STACK_SIZE; idx++)
	{
		const ltc_crv_t* chip = &ltc_voltages[idx];
		for (int8_t cell=0; cell<12; cell++)
		{
			uint16_t voltage = ltc_get_voltage_internal(chip, cell);
			if (voltage > max)
			{
				max = voltage;
			}
		}
	}

	os_exitCS();

	return ltc_adc_voltage(max);
}

uint16_t ltc_get_min_voltage(void)
{
	uint16_t min=32768;

	os_enterCS();
	
	for (int8_t idx=0; idx<LTC_STACK_SIZE; idx++)
	{
		const ltc_crv_t* chip = &ltc_voltages[idx];
		for (int8_t cell=0; cell<12; cell++)
		{
			uint16_t voltage = ltc_get_voltage_internal(chip, cell);
			if (voltage < min)
			{
				min = voltage;
			}
		}
	}

	os_exitCS();

	return ltc_adc_voltage(min);
}

void ltc_set_load(const int8_t cell, const uint8_t status)
{
	int8_t block, j;

	for (j=12, block=0; j<96; j+=12, block++)
	{
		if (cell<j)
		{
			break;
		}
	}
	
	uint16_t mask = 1<<(cell+12-j);
	uint16_t dcc;

	os_enterCS();
	dcc = ltc_config[block].dcc;
	if (status)
	{
		dcc |= mask;
	}
	else
	{
		dcc &= ~mask;
	}
	ltc_config[block].dcc = dcc;
	os_exitCS();
}

uint8_t ltc_poll_interrupt(uint16_t ms)
{
	uint8_t last8, now8;
	uint16_t filter=1;

	ltc_chip_select(TRUE);
	spi_master_transmit(PLINT);

	uint32 start = TIMER_GET_TICKCOUNT_32;
	last8 = TIMER_GET_TICKCOUNT_8;
	if ((uint8)start != last8) start = TIMER_GET_TICKCOUNT_32;
	uint32 ticksToWait = MS_TO_TICKS((uint32)ms);
	uint32 now;
	do {
		now = TIMER_GET_TICKCOUNT_32;
		now8 = TIMER_GET_TICKCOUNT_8;
		if ((uint8)now != now8) now = TIMER_GET_TICKCOUNT_32;
		if (last8 != now8)
		{
			// every 400us
			filter <<= 1;
			if (PINB & LTC_SDI)
			{
				filter |= 0x1;;
			}
			if (filter == 0xffff)
			{
				// cable break!
				ltc_chip_select(FALSE);
				return LTC_POLL_DISCONNECTED;
			}
			if (filter == 0)
			{
				// interrupt!
				ltc_chip_select(FALSE);
				return LTC_POLL_INTERRUPT;
			}
			last8 = now8;
		}
	} while (now-start < ticksToWait);
	ltc_chip_select(FALSE);

	return LTC_POLL_OK;
}

uint8_t ltc_poll_adc(uint8_t ms)
{
	uint8_t last8, now8;
	uint8_t filter=1;

	ltc_chip_select(TRUE);
	spi_master_transmit(PLADC);

	uint32 start = TIMER_GET_TICKCOUNT_32;
	last8 = TIMER_GET_TICKCOUNT_8;
	if ((uint8)start != last8) start = TIMER_GET_TICKCOUNT_32;
	uint32 ticksToWait = MS_TO_TICKS((uint32)ms);
	uint32 now;
	do {
		now = TIMER_GET_TICKCOUNT_32;
		now8 = TIMER_GET_TICKCOUNT_8;
		if ((uint8)now != now8) now = TIMER_GET_TICKCOUNT_32;
		if (last8 != now8)
		{
			// every 400us
			filter <<= 1;
			if (PINB & LTC_SDI)
			{
				filter |= 0x1;;
			}
			if (filter == 0xff)
			{
				// cable break!
				ltc_chip_select(FALSE);
				return LTC_POLL_DISCONNECTED;
			}
			if ((filter & 0xe0) == 0xa0)
			{
				// interrupt!
				ltc_chip_select(FALSE);
				return LTC_POLL_ADC;
			}
			last8 = now8;
		}
	} while (now-start < ticksToWait);
	ltc_chip_select(FALSE);

	return LTC_POLL_TIMEOUT;
}

void ltc_update_data()
{
	uint8_t ret;

	ltc_start_temperature_conversion();
	ret = ltc_poll_adc(100);
	if (ret == LTC_POLL_ADC)
	{
		ltc_read_temperature();
	}

	ltc_start_voltage_conversion();
	ret = ltc_poll_adc(100);
	if (ret==LTC_POLL_ADC)
	{
		ltc_read_voltage();
	}
}

inline uint16_t ltc_adc_voltage(uint16_t adc)
{
	return adc + (adc>>1);
}

void ltc_calc_ber(void)
{
	uint32_t ber = ltc_bytes_bad;
	ber *= 10000;
	ber /= ltc_bytes_ok;

	data_spi_ber = ber;

	if (ltc_bytes_ok>64000)
	{
		ltc_bytes_ok = 1; // Avoid div 0.
		ltc_bytes_bad = 0;
	}
}
