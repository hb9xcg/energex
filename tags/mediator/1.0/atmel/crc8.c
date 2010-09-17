/*
This code is from Colin O'Flynn - Copyright (c) 2002
only minor changes by: M.Thomas 9/2004 and M.Walser 3/2009

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


#include <inttypes.h>

#define CRC8INIT        0x00
#define CRC8POLY        0x18              //0X18 = X^8+X^5+X^4+X^0

uint8_t crc8 (uint8_t data[], uint8_t nbr_of_bytes)
{
        uint8_t  byteIdx, bitIdx, crc = CRC8INIT;

        for (byteIdx = 0; byteIdx != nbr_of_bytes; byteIdx++)
        {
                uint8_t dataByte = data[byteIdx];

                for (bitIdx=0; bitIdx<8; bitIdx++)
                {
                        uint8_t feedback_bit = (crc ^ dataByte) & 0x01;

                        if ( feedback_bit == 0x01 )
                        {
                                crc ^= CRC8POLY;
                        }
                        crc = (crc >> 1) & 0x7F;
                        if ( feedback_bit == 0x01 )
                        {
                                crc |= 0x80;
                        }

                        dataByte >>= 1;
                }
        }

        return crc;
}

