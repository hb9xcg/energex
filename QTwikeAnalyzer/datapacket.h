#ifndef DATAPACKET_H
#define DATAPACKET_H

#include <QTime>
#include <QByteArray>
#include <QStandardItemModel>
#include <QtGlobal>

class QStandardItem;

class DataPacket : public QObject
{
    Q_OBJECT
    Q_ENUMS(EType)
    Q_ENUMS(EChecksum)

    QStandardItem* timeItem;
    QStandardItem* rawItem;
    QStandardItem* addressItem;
    QStandardItem* typeItem;
    QStandardItem* contentItem;
    QStandardItem* checksumItem;

public:


public:
    DataPacket();
    void setTimeData(QTime time);
    void setRawData(QByteArray raw);
    void setAddressData(quint8 address);
    void setTypeData(EType currentType);
    void setContentData(QString content);
    void setChecksumData(EChecksum checksum);
    void updateModel(QStandardItemModel* model);
};

#endif // DATAPACKET_H
