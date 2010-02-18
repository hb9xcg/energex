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
