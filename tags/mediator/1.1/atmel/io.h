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
 * @file 	io.h
 * @brief 	board specific io port configuration.
 * @author 	Markus Walser (markus.walser@gmail.com)
 * @date 	15.11.08
 */

#ifndef IO_H_DEF
#define IO_H_DEF

#define TRANSMIT_ENABLE 0x08	// Port B

void io_init(void);

void io_raise_emergency(void);
void io_release_emergency(void);

void io_enable_interface_power(void);
void io_disable_interface_power(void);

void io_enable_igbt(void);
void io_disable_igbt(void);

void io_close_relais(void);
void io_open_relais(void);
uint8_t io_is_relais_closed(void);

void io_set_green_led(void);
void io_clear_green_led(void);
void io_toggle_green_led(void);

void io_set_red_led(void);
void io_clear_red_led(void);
void io_toggle_red_led(void);

void io_enable_stop_interrupt(void);
uint8_t io_get_stop(void);
#endif
