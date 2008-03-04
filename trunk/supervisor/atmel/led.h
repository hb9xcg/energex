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
 * @file 	led.h
 * @brief 	Contains some LED macros.
 * @author 	Markus Walser (markus.walser@gmail.com)
 * @date 	04.03.2008
 */
#define ON          1
#define OFF         0

#define RED(on)     if(on) PORTB |= 0x08; else PORTB &= ~0x08;
#define YELLOW(on)  if(on) PORTB |= 0x00; else PORTB &= ~0x00;
#define GREEN(on)   if(on) PORTB |= 0x10; else PORTB &= ~0x10;
#define I2C(on)     if(on) PORTB |= 0x20; else PORTB &= ~0x20;
#define POWER(on)   if(on) PORTB |= 0x40; else PORTB &= ~0x40;
