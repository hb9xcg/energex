/***************************************************************************
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
#include "checksum.h"

Checksum::Checksum()
{
}

// checksum of CRC16
int Checksum::crc16(char buffer[], int length, int Mask, short Init)
{
    int  crc;
    int  i, t;

    crc = Init;
    for (i=0; i<length; i++)
    {
        t = buffer[i] * 256;
        crc = (crc ^ t);
        for (t=0; t<8; t++)
        {
            if ((crc & 0x8000) == 0x8000)
            {
                crc = (short) ((crc * 2) ^ Mask);
            }
            else
            {
                crc = (short) (crc * 2);
            }
            crc &= 0xFFFF;
        }
    }
    return crc;
}

//checksum of accumulate
int Checksum::sumChecksum(char buf[], int length, int Mask, short Init)
{
    int crc;
    int i;

    crc = Init;
    for (i=0; i<length; i++)
    {
        crc += buf[i];
        crc &= 0xFFFF;
    }

    return crc;
}
