#ifndef CHECKSUM_H
#define CHECKSUM_H

#include <QtGlobal>

class Checksum
{
public:
    Checksum();
    static int crc16(char buffer[], int length, int Mask, short Init);
    static int sumChecksum(char buf[], int length, int Mask, short Init);
};

#endif // CHECKSUM_H
