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
 * @file 	supervisor.h
 * @brief 	Collects information from each supervisor slave.
 * @author 	Markus Walser (markus.walser@gmail.com)
 * @date 	11.02.08
 */
#ifndef _SUPERVISOR_H
#define _SUPERVISOR_H

#include "mediator.h"

typedef struct {
	int16_t  voltage;      // [mV]
	int8_t   temperture;   // [°C]
	uint8_t  load;         // [0...100%]
	uint8_t  error;        // 0x1 send | 0x2 recv
} cell_t;

void supervisor_init(void);
void supervisor_activate(void);
void supervisor_deactivate(void);
int8_t supervisor_get_nbr_of_info(void);
void supervisor_get_info(int8_t idx, cell_t* info);

#endif
