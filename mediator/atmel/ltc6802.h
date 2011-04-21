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

#ifndef LTC6802_H_DEF
#define LTC6802_H_DEF

#include "mediator.h"

/*! 
 * @file        ltc6802.h
 * @brief       Battery Stack Monitor driver.
 * @author      Markus Walser (markus.walser@gmail.com)
 * @date        14.11.2009
 */


#define LTC_STACK_SIZE 4

typedef union 
{
        struct 
	{
                uint8_t cdc:3;		/*!< CDC bits in CFGR0 */
                uint8_t cell10:1;	/*!< CELL10 bit in CFGR0 */
                uint8_t lvlpl:1;	/*!< LVLPL bit in CFGR0 */
		uint8_t gpio1:1;	/*!< GPIO1 bit in CFGR0 */
		uint8_t gpio2:1;	/*!< GPIO2 bit in CFGR0 */
		uint8_t wdten:1;	/*!< WDTEN bit in CFGR0 */
		uint16_t dcc:12;	/*!< DDC bits in CFGR1 and CFGR2 */
		uint16_t mci:12;	/*!< MCI bits in CFGR2 and CFGR3 */
		uint8_t vuv:8;		/*!< VUV bits in CFGR3 and CFGR4 */
		uint8_t vov:8;		/*!< VOV bits in CFGR3 and CFGR4 */
        } __attribute__((packed));
        uint8_t cfgr[6];    /*!< All bits as raw data */
} ltc_config_t;  /*!< Configuration for ltc6802 */

typedef union 
{
	struct 
	{
		uint16_t c1v:12;
		uint16_t c2v:12;
		uint16_t c3v:12;
		uint16_t c4v:12;
		uint16_t c5v:12;
		uint16_t c6v:12;
		uint16_t c7v:12;
		uint16_t c8v:12;
		uint16_t c9v:12;
		uint16_t c10v:12;
		uint16_t c11v:12;
		uint16_t c12v:12;
	} __attribute__((packed));
	uint8_t crv[18];
} ltc_crv_t;

typedef union
{
	struct
	{
		uint8_t c1uv:1;
		uint8_t c1ov:1;
		uint8_t c2uv:1;
		uint8_t c2ov:1;
		uint8_t c3uv:1;
		uint8_t c3ov:1;
		uint8_t c4uv:1;
		uint8_t c4ov:1;
		uint8_t c5uv:1;
		uint8_t c5ov:1;
		uint8_t c6uv:1;
		uint8_t c6ov:1;
		uint8_t c7uv:1;
		uint8_t c7ov:1;
		uint8_t c8uv:1;
		uint8_t c8ov:1;
		uint8_t c9uv:1;
		uint8_t c9ov:1;
		uint8_t c10uv:1;
		uint8_t c10ov:1;
		uint8_t c11uv:1;
		uint8_t c11ov:1;
		uint8_t c12uv:1;
		uint8_t c12ov:1;
	} __attribute__((packed));
	uint8_t flg[3];
} ltc_flg_t;

typedef union 
{
	struct 
	{
		uint16_t etmp1:12;
		uint16_t etmp2:12;
		uint16_t itmp:12;
		uint16_t thsd:1;
		uint16_t rev:3;
	} __attribute__((packed));
	uint8_t tmpr[5];
} ltc_tmpr_t;

// prototypes
extern void ltc_init(void);
extern void ltc_set_standby_mode(void);
extern void ltc_write_config(void);
extern void ltc_start_voltage_conversion(void);
extern void ltc_read_voltage(void);
extern void ltc_start_temperature_conversion(void);
extern void ltc_read_temperature(void);
extern int16_t ltc_get_internal_temperature(uint8_t index);
extern int16_t ltc_get_external_temperature(uint8_t index);
extern void ltc_get_temperature_min_avg_max(int16_t *min, int16_t *avg, int16_t *max);
extern int16_t ltc_get_temperature_max(void);
extern int16_t ltc_get_temperature_min(void);

extern uint16_t ltc_get_voltage(const uint8_t cell); 
extern void ltc_get_voltage_limits(uint16_t* out_min, uint16_t* out_max);
extern void ltc_get_voltage_min_avg_max(int16_t* out_min, int16_t* out_avg, int16_t* out_max);
extern uint16_t ltc_get_max_voltage(void);
extern uint16_t ltc_get_min_voltage(void);

extern void ltc_set_load(const int8_t cell, const uint8_t status); 

extern uint16_t ltc_adc_voltage(uint16_t adc);

#define LTC_POLL_DISCONNECTED	2
#define LTC_POLL_INTERRUPT	1
#define LTC_POLL_OK		0
uint8_t ltc_poll_interrupt(uint16_t ms);

#define LTC_POLL_ADC		3
#define LTC_POLL_TIMEOUT	4
uint8_t ltc_poll_adc(uint8_t ms);

void ltc_update_data(void);

#endif
