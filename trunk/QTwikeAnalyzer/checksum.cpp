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
