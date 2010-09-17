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
 * @file 	balancer.c
 * @brief 	Contains balancing controller.
 * @author 	Markus Walser (markus.walser@gmail.com)
 * @date 	12.02.08
 */

#include "balancer.h"
#include "supervisor.h"
#include "charge.h"

#define BALANCER_MAX_CELL_VOLTAGE	4100 // We charge just to 4100mV
#define BALANCER_MIN_CURRENT		 500 // We do not want to charge with current less than 500mA

enum EBlancerState
{
	eConstantI,
	eConstantU,
	eBalancing,
	eDischarging
};
static enum EBlancerState eSupervisorState = eDischarging;

/* Prototypes */
void balance_i(void);
void balance_u(void);
void balance_sym(void);

// Will be called every 10s
void balance_sample(void)
{
	switch(eSupervisorState)
	{
	case eConstantI:
		balance_i();
		break;
	case eConstantU:
		balance_u();
		break;
	case eBalancing:
		balance_sym();
		break;
	case eDischarging:
		break;
	}
}

uint8_t balance_max_voltage(void)
{
	cell_t cell;
	int8_t idx, nbrOfCells=supervisor_get_nbr_of_info();
	
	for( idx=0; idx<nbrOfCells; idx++)
	{
		supervisor_get_info(idx, &cell);
		if( cell.voltage >= BALANCER_MAX_CELL_VOLTAGE )
		{
			return 1;
		}
	}
	return 0;
}

uint8_t balance_min_current(void)
{
	int16_t current;
	
	current = charge_get_current();
	
	return( current <= BALANCER_MIN_CURRENT );
}

uint8_t balance_all_cell_full(void)
{
	return 1; // TODO
}

void balance_i(void)
{
	if( balance_max_voltage() )
	{
		eSupervisorState = eConstantU;
	}
	else
	{
		// TODO
	}
}

void balance_u(void)
{
	if( balance_min_current() )
	{
		eSupervisorState = eBalancing;
	}
	else
	{
		int slave, nbrOfCells=supervisor_get_nbr_of_info();
	
		for( slave=1; slave<=nbrOfCells; slave++)
		{
			// TODO
		}
	}
}

void balance_sym(void)
{
	if( balance_all_cell_full() )
	{
		eSupervisorState = eDischarging;
	}
	else
	{
		// TODO
	}
}
