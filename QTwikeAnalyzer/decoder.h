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
#ifndef DECODER_H
#define DECODER_H

#include <QObject>
#include <QTime>
#include "protocol.h"

class QRegExp;

class Decoder : public QObject
{
    Q_OBJECT
    Q_ENUMS(EPacketState)
    Q_ENUMS(EType)
    Q_ENUMS(EChecksum)
    Q_ENUMS(EUnit)

public:
    enum EPacketState
    {
        eExpectStart,
        eExpectAddress,
        eExpectCommand,
        eExpectData,
        eExpectChecksum
    };

    enum EType
    {
        RxGroup,
        RxData,
        TxGroup,
        TxData
    };

    enum EChecksum
    {
        Valid,
        Invalid
    };

    enum EUnit
    {
        eVoltage,
        eCurrent,
        eCharge,
        eTemperatur,
        ePower,
        eSpeed,
        eDistance,
        eUnknown
    };

private:
    QTime* time;
    EPacketState eState;
    quint8 command;
    quint8 checksum;
    quint8 length;
    qint8 idx;
    quint8 address;
    quint8 parameter;
    quint8 data[8];
    bool frameDetection;

    QRegExp* regSpannung;
    QRegExp* regStrom;
    QRegExp* regTemperatur;
    QRegExp* regCharge;
    QRegExp* regPower;
    QRegExp* regSpeed;
    QRegExp* regDistance;

    void receivePacket(void);
    void receiveRubish(quint8 lenght, quint8 checksum);
    void requestData(void);
    void requestGroup(void);
    void transmitData(void);
    void transmitGroup(void);

    QString decodeAddress(quint8 address);
    QString decodeParameter(quint16 parameter);
    QString decodeValue(quint16 parameter, quint16 unsignedValue);
    QString decodeUnit(EUnit eUnit);
    QString decodeDriveState(EDriveState state);
    QString decodeBInfo(quint16 binfo);
    EUnit getUnit(quint8 parameter);

public:
    Decoder();
    ~Decoder();
    void resetTime();

public slots:
    void receiveByte(const char byte);

signals:
    void receiveMessage(QString time, QString raw, QString address, QString type, QString content, QString checksum);
    void battery1(int time, float voltage, float current);
    void temperature1(int time, int sensor, float temperature);
    void battery2(int time, float voltage, float current);
    void temperature2(int time, int sensor, float temperature);
    void battery3(int time, float voltage, float current);
    void temperature3(int time, int sensor, float temperature);
};

#endif // DECODER_H
