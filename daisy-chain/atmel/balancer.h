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
 * @file 	balancer.h
 * @brief 	Contains balancing controller.
 * @author 	Markus Walser (markus.walser@gmail.com)
 * @date 	12.02.08
 */
#ifndef _BALANCER_H
#define _BALANCER_H

#include "mediator.h"

#define BALANCER_NBR_OF_CELLS             (LTC_STACK_SIZE*12)
extern void balancer_init(void);

#define BALANCER_STANDBY	0
#define BALANCER_SURVEILLANCE	1
#define BALANCER_ACTIVE 	2
#define BALANCER_OFF	 	3

extern void balancer_set_state(uint8_t state);
extern void balance_sample(void);
extern uint16_t balancer_get_max_stack_usage(void);
extern void balancer_dump(void);


#endif
