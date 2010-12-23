
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


#ifndef ERROR_H_DEF
#define ERROR_H_DEF
/*! 
 * @file 	error.h
 * @brief 	Error handling.
 * @author 	Markus Walser (markus.walser@gmail.com)
 * @date 	19.09.2010
 */

#define ERROR_CRC		1
#define ERROR_CABLE_BREAK	2
#define ERROR_UNKNOWN_PARAMETER	3
#define ERROR_DS1307_WRITE	4
#define ERROR_DS1307_READ	5
#define ERROR_INVALID_DSTATE	6

void fatal(uint8_t code);
void error(uint8_t code);
void warning(void);

#endif
