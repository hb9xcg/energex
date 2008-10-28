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
 * @file 	simulator.c
 * @brief 	Simulates RS485 communication to test the QTwikeAnalyzer.
 * @author 	Markus Walser (markus.walser@gmail.com)
 * @date 	27.03.08
 */

#include "simulator.h"
#include "i2c.h"
#include "os_thread.h"
#include "uart.h"

#define SIMULATOR_STACK_SIZE  128

uint8_t simulator_stack[SIMULATOR_STACK_SIZE];
static Tcb_t * simulator_thread;
static int8_t active;

// Internal prototypes
static void simulator(void);


void simulator_init(void)
{
	simulator_thread = os_create_thread((uint8_t *)&simulator_stack[SIMULATOR_STACK_SIZE-1], simulator );
}

void simulator_activate()
{
	active = 1;         // activate supervisor thread
}

void simulator_deactivate()
{
	active = 0;
}

void simulator(void)
{	
	static const uint8_t request[]={0x10, 0x31, 0x42, 0x27, 0x65};
	static const uint8_t response[]={0x10, 0x31, 0x48, 0x80, 0xff, 0xfd, 0x87, 0xd8, 0x36, 0xa3};
	for(;;)
	{
		if(active)
		{
			uart_write( request, sizeof(request) );
			os_thread_sleep(100);
			uart_write( response, sizeof(response) );
		}
		os_thread_sleep(900);
	}
}
