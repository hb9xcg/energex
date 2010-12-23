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
 * @file 	idle.c
 * @brief 	Idle task.
 * @author 	Markus Walser (markus.walser@gmail.com)
 * @date 	24.10.2010
 */
#include "idle.h"
#include "io.h"
#include <string.h>
#include "os_thread.h"
#include "delay.h"

#define IDLE_STACK_SIZE 128

uint8_t idle_stack[IDLE_STACK_SIZE];
static Tcb_t * idle_thread;
static void idle_task(void);


void idle_init(void)
{
	memset(idle_stack, 0xb6, sizeof(idle_stack));
	idle_thread = os_create_thread((uint8_t *)&idle_stack[IDLE_STACK_SIZE-1], 
			idle_task);
}

uint16_t idle_get_max_stack_usage(void)
{
	uint16_t stack_idx = 0;
	while(stack_idx < IDLE_STACK_SIZE)
	{
		if (idle_stack[stack_idx] != 0xb6)
		{
			break;
		}
		stack_idx++;
	}
	return IDLE_STACK_SIZE-stack_idx;
}


void idle_task(void)
{
	for(;;)
	{
	}
}

