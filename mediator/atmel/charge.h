/***************************************************************************
 *   Energex                                                               *
 *                                                                         *
 *   Copyright (C) 2008-2010 by Markus Walser                              *
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
 * @file 	charge.h
 * @brief 	Measures and integrates current.
 * @author 	Markus Walser (markus.walser@gmail.com)
 * @date 	11.02.08
 */

#ifndef _CHARGE_H_
#define _CHARGE_H_

#include "mediator.h"

int16_t charge_get_current(void);

void charge_sample(void);
int16_t charge_get_capacity(void);
void charge_set_capacity(int16_t newCapacity);
void charge_reset(void);
void charge_start(void);
void charge_set_total_charge(uint16_t new_total);
uint16_t charge_get_total_charge(void);
void charge_set_total_discharge(uint16_t new_total);
uint16_t charge_get_total_discharge(void);
int32_t charge_get_barrel(void);
void charge_set_barrel(int32_t barrel);

uint16_t charge_get_counter_charged_Ah(void);
uint16_t charge_get_counter_discharged_Ah(void);
void charge_set_counter_charged_Ah(uint16_t counter_charged_Ah);
void charge_set_counter_discharged_Ah(uint16_t counter_discharged_Ah);

#endif
