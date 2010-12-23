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
 * @file 	data.h
 * @brief 	Stores and loads persistent data.
 * @author 	Markus Walser (markus.walser@gmail.com)
 * @date 	22.02.08
 */

#ifndef _DATA_H_
#define _DATA_H_

#include "global.h"

void data_save(void);
void data_load(void);

extern int16_t data_capacity;
extern uint16_t data_total_discharge;
extern uint16_t data_total_discharge;

typedef union
{
	struct
	{
		uint16_t cycles_under_10; // <  10%
		uint16_t cycles_over_10;  // >  10%
		uint16_t cycles_over_20;  // >  20%
		uint16_t cycles_over_30;  // >  30%
		uint16_t cycles_over_40;  // >  40%
		uint16_t cycles_over_50;  // >  50%
		uint16_t cycles_over_60;  // >  60%
		uint16_t cycles_over_70;  // >  70%
		uint16_t cycles_over_80;  // >  80%
		uint16_t cycles_over_90;  // >  90%
		uint16_t cycles_over_100; // > 100%
		uint16_t cycles_over_110; // > 110%
	};
	uint16_t cycles[12];
} data_stat_t;
extern data_stat_t data_stat;

extern uint16_t data_charge_cycles;
extern uint16_t data_deep_discharge_cycles;

extern uint16_t data_nominal_capacity;  
extern uint16_t data_max_capacity;  
extern uint16_t data_min_capacity;  


#endif
