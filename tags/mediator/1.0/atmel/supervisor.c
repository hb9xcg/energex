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
 * @file 	supervisor.c
 * @brief 	Collects information from each supervisor slave.
 * @author 	Markus Walser (markus.walser@gmail.com)
 * @date 	11.02.08
 */

#include "supervisor.h"
#include "i2c.h"
#include "os_thread.h"
#include "delay.h"

#define SUPERVISOR_STACK_SIZE  256
#define NUMBER_OF_CELLS        100
#define SUPERVISOR_POWER      0x10
#define SUPERVISOR_WAKEUP     0x80
#define ERROR_SEND            0x01
#define ERROR_RECV            0x02
#define ERROR_OFF             0x04



uint8_t supervisor_stack[SUPERVISOR_STACK_SIZE];
static Tcb_t * supervisor_thread;
static int8_t active;
static cell_t power_pack[NUMBER_OF_CELLS];

static void supervisor(void);
static void measure_voltage(uint8_t slave);
static uint16_t read_voltage(uint8_t slave);


void supervisor_init(void)
{
	DDRD |= SUPERVISOR_POWER;  // Enable bus-power output
	DDRD |= SUPERVISOR_WAKEUP; // Enable wakeup output

	supervisor_thread = os_create_thread((uint8_t *)&supervisor_stack[SUPERVISOR_STACK_SIZE-1], supervisor );
}

void supervisor_activate()
{
	PORTD |= SUPERVISOR_POWER;		// Powerup I2C bus
	os_thread_sleep(500);

	PORTD |= SUPERVISOR_WAKEUP;		// Wakeup supervisor slave boards
	os_thread_sleep(200);
	PORTD &= ~SUPERVISOR_WAKEUP;

	active = 1;         // activate supervisor thread
}

void supervisor_deactivate()
{
	active = 0;

	os_thread_sleep(1000);

	PORTD &= ~SUPERVISOR_POWER;       // Powerdown I2C bus
}

void supervisor(void)
{
	int slave;
	
	for(;;)
	{
		if(active)
		{
			for( slave=89; slave<=89; slave++)
			{
				if(!active)
					break;
				power_pack[slave-1].error &= ~ERROR_OFF;
				measure_voltage(slave);
				if( i2c_wait() != TW_NO_INFO)
				{
					// send error
					power_pack[slave-1].error |= ERROR_SEND;
				}
				else
				{
					power_pack[slave-1].error &= ~ERROR_SEND;
				}
			}
			
			delay(1000);
			
			for( slave=89; slave<=89; slave++)
			{
				if(!active)
					break;
				power_pack[slave-1].voltage = read_voltage(slave);
				if( i2c_wait() != TW_NO_INFO)
				{
					// receive error
					power_pack[slave-1].error |= ERROR_RECV;
				}
				else
				{
					power_pack[slave-1].error &= ~ERROR_RECV;
				}
			}
		}
		else
		{
			for( slave=1; slave<=NUMBER_OF_CELLS; slave++)
			{
				power_pack[slave-1].error |= ERROR_OFF;
				power_pack[slave-1].voltage = 0;
			}
		}
		delay(1000);
	}
}

void measure_voltage(uint8_t slave)
{
	uint8_t recv_msg[0];
	uint8_t send_msg[] = { 'u' };
	
	// write u, read nothing
	i2c_write_read( slave, send_msg, sizeof(send_msg), recv_msg, sizeof(recv_msg) );
}

uint16_t read_voltage(uint8_t slave)
{
	uint8_t send_msg[0];
	uint8_t recv_msg[6];

	// write nothing, read voltage
	i2c_write_read( slave, send_msg, sizeof(send_msg), recv_msg, sizeof(recv_msg) );

	uint16_t voltage = 0;

	voltage += 1000*(recv_msg[0]-'0');
	voltage +=  100*(recv_msg[1]-'0');
	voltage +=   10*(recv_msg[2]-'0');
	voltage +=    1*(recv_msg[3]-'0');

	return voltage;
}


void measure_temperatur(uint8_t slave)
{
	uint8_t recv_msg[0];
	uint8_t send_msg[] = { 't' };
	
	// write t, read nothing
	i2c_write_read( slave, send_msg, sizeof(send_msg), recv_msg, sizeof(recv_msg) );
}

int8_t read_temperatur(uint8_t slave)
{
	uint8_t send_msg[0];
	uint8_t recv_msg[5];

	// write nothing, read temperature
	i2c_write_read( slave, send_msg, sizeof(send_msg), recv_msg, sizeof(recv_msg) );

	int8_t temperatur = 0;

	temperatur += 10*(recv_msg[1]-'0');
	temperatur +=  1*(recv_msg[2]-'0');

	if(recv_msg[0] == '-')
	{
		temperatur = -temperatur;
	}

	return temperatur;
}


void set_load(uint8_t slave, uint8 load)
{
	uint8_t send_msg[1];
	uint8_t recv_msg[0];

	load += 5;
	load /= 10;

	if( load < 10)
	{
		send_msg[0] = '0' + load;
	}
	else
	{
		send_msg[0] = 'a';
	}

	// write load, read nothing
	i2c_write_read( slave, send_msg, sizeof(send_msg), recv_msg, sizeof(recv_msg) );
}

void supervisor_get_info(int8_t idx, cell_t* info)
{
	if( idx < NUMBER_OF_CELLS)
	{
		// TODO May be we copy the address later instead the values.
		*info = power_pack[idx];
	}
}

int8_t supervisor_get_nbr_of_info(void)
{
	return NUMBER_OF_CELLS;
}


