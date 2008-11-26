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
 * @file 	io.h
 * @brief 	board specific io port configuration.
 * @author 	Markus Walser (markus.walser@gmail.com)
 * @date 	15.11.08
 */

#ifndef IO_H_DEF
#define IO_H_DEF

#define IGBT   		0x04
#define RELAIS 		0x02
#define STOP_SWITCH	0x04
#define LED_RED		0x08	// Port C
#define LED_GREEN	0x10	// Port C
#define TRANSMIT_ENABLE 0x08	// Port B
#define POWER_ONE_WIRE  0x10	// Port D

#endif
