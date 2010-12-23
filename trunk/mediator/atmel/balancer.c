/*
 * Energex
 * 
 * Copyright (C) 2008-2010 by Markus Walser
 * markus.walser@gmail.com
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
 * @file 	balancer.c
 * @brief 	Contains balancing controller.
 * @author 	Markus Walser (markus.walser@gmail.com)
 * @date 	12.02.08
 */

#include <string.h>
#include "balancer.h"
#include "os_thread.h"
#include "delay.h"
#include "ltc6802.h"
#include "error.h"
#include "io.h"
//#define DEBUG
#ifdef DEBUG
  #include <uart.h>
  #include <stdlib.h>
  #include <stdio.h>
//  #include <avr/pgmspace.h>
#endif

#ifdef DEBUG
    static uint8_t balancer_debug[16];

    #define TRACE(x)        sprintf((char*)balancer_debug, "%d\r\n", (x)); \
                            uart_write(balancer_debug, strlen((char*)balancer_debug));
#else
    #define TRACE(x)
#endif

#define BALANCER_STACK_SIZE              160
#define BALANCER_MAX_CELL_VOLTAGE       4150 // We charge just to 4.10mV
#define BALANCER_MIN_CURRENT            2500 // End of charge current 2.5A

static uint8_t balancer_stack[BALANCER_STACK_SIZE];
static Tcb_t * balancer_thread;
static int8_t  balancer_load[BALANCER_NBR_OF_CELLS];
static int8_t  balancer_state = BALANCER_SURVEILLANCE;

static void balancer_loop(void);
static void balancer_recalculate(void);
static void balancer_switch_load(void);
static void balancer_all_off(void);
static void balancer_check_voltage(void);
static void balancer_twike_report(void);


void balancer_init(void)
{
	memset(balancer_stack, 0xb6, sizeof(balancer_stack));
	balancer_thread = os_create_thread( (uint8_t *)&balancer_stack[BALANCER_STACK_SIZE-1], 
	                                    balancer_loop );
}

uint16_t balancer_get_max_stack_usage(void)
{
	uint16_t stack_idx = 0;
	while(stack_idx < BALANCER_STACK_SIZE)
	{
		if (balancer_stack[stack_idx] != 0xb6)
		{
			break;
		}
		stack_idx++;
	}
	return BALANCER_STACK_SIZE-stack_idx;
}

void balancer_set_state(uint8_t on)
{
	balancer_state = on;
}

void balancer_loop(void)
{
	static uint8_t counter;

	ltc_init();
	
	for (;;)
	{

		switch (balancer_state)
		{
		case BALANCER_ACTIVE:
		{
			ltc_update_data();
			balancer_twike_report();

			if (counter == 0)
			{
				counter = 16;
				balancer_recalculate();
			}
			counter--;
		
			balancer_switch_load();
			
			io_toggle_green_led();
			
			balancer_check_voltage();
			break;
		}

		case BALANCER_SURVEILLANCE:
		{
			ltc_update_data();
			balancer_twike_report();

			if (counter != 0)
			{
				counter = 0;
				io_clear_green_led();
				balancer_all_off();
			}
			balancer_check_voltage();
			break;
		}
		
		case BALANCER_STANDBY:
			ltc_set_standby_mode();
			balancer_state = BALANCER_OFF;
			break;

		case BALANCER_OFF:
			break;

		default:
			break;
		}
	}
}

void balancer_twike_report(void)
{
	int16_t min, avg, max;
	ltc_get_voltage_min_avg_max(&min, &avg, &max);

	battery_set_parameter_value(SYM_SPANNUNG, BATTERY_1, ltc_adc_voltage(min)/10);
	battery_set_parameter_value(SYM_SPANNUNG, BATTERY_2, ltc_adc_voltage(avg)/10);
	battery_set_parameter_value(SYM_SPANNUNG, BATTERY_3, ltc_adc_voltage(max)/10);


	ltc_get_temperature_min_avg_max(&min, &avg, &max);

	if (balancer_state==BALANCER_ACTIVE)
	{
		// Report 50°C higher temperature to switch on fan of battery 1 and 3:
		min += 5000;
		max += 5000;
	}

	battery_set_parameter_value(BATTERIE_TEMP, BATTERY_1, min);
	battery_set_parameter_value(BATTERIE_TEMP, BATTERY_2, avg);
	battery_set_parameter_value(BATTERIE_TEMP, BATTERY_3, max);
}

void balancer_check_voltage(void)
{
	static uint8_t errors;
	switch (ltc_poll_interrupt(500))
	{
		case LTC_POLL_INTERRUPT:
			// Under voltage or over voltage condition!
			mediator_cell_limit_reached();
			break;

		case LTC_POLL_DISCONNECTED:
			errors++;
			if (errors>10)
			{
				error(ERROR_CABLE_BREAK);
			}
			break;

		default: 
			errors=0;
			mediator_cell_limit_ok();
	}
}

void balancer_recalculate(void)
{
	uint16_t min_voltage, max_voltage;

	ltc_get_voltage_limits( &min_voltage, &max_voltage);

	min_voltage <<= 4;
	max_voltage <<= 4;

	uint16_t gap = max_voltage-min_voltage;
        gap >>= 4;

	for (uint8_t cell=0; cell<BALANCER_NBR_OF_CELLS; cell++)
	{
		uint16_t cell_voltage = ltc_get_voltage(cell);
		cell_voltage <<= 4;

		if (gap > 0)
		{
			uint16_t voltage;
			uint8_t load = 0;
			for ( voltage=min_voltage; voltage<=max_voltage; voltage+=gap)
			{
				if (cell_voltage >= voltage && 
				    cell_voltage <  voltage+gap	)
				{
					balancer_load[cell] = load;
					break;
				}
				load++;
			}
		}
		else
		{
			balancer_load[cell] = 0;
		}	
	}
}

void balancer_switch_load(void)
{
	
	for (uint8_t cell=0; cell<BALANCER_NBR_OF_CELLS; cell++)
	{
		if (balancer_load[cell] > 0)
		{
			balancer_load[cell]--;
			ltc_set_load(cell, 1);
		}
		else
		{
			TRACE(cell);
			ltc_set_load(cell, 0);
		}
	}
	ltc_write_config();
}
				
void balancer_all_off(void)
{
	for (uint8_t cell=0; cell<BALANCER_NBR_OF_CELLS; cell++)
	{
		ltc_set_load(cell, 0);
	}
	ltc_write_config();
}

#ifdef DEBUG
void balancer_dump()
{
	char temp[6], line[16];
	for (uint8_t cell=0; cell<BALANCER_NBR_OF_CELLS; cell++)
	{
	        strcpy(line, itoa(balancer_load[cell], temp, 10));
		strcat(line, "\r\n");
		uart_write((uint8_t*)line, strlen(line));
	}
}
#endif

