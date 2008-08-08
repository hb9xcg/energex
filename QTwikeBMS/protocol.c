/***************************************************************************
 *   Copyright (C) 2008 by Markus Walser                                   *
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
#include "protocol.h"

#define START_DATA_IDX		5

uint8_t temp[16];

uint8_t frame_stuffing(uint8_t packet[], uint8_t length)
{
	int8_t idxFrom, idxTo;
	for(idxFrom=START_DATA_IDX; idxFrom<length; idxFrom++)
	{
		if(packet[idxFrom] == FRAME)
		{
			for(idxTo=length; idxTo>idxFrom; idxTo--)
			{
				packet[idxTo] = packet[idxTo-1];
			}
			length++;
			idxFrom++;
		}
	}
	return length;
}