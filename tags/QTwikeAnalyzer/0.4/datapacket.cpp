#include "datapacket.h"
#include "ihexconverter.h"


DataPacket::DataPacket()
{
}

void DataPacket::setTimeData(QTime time)
{
    timeItem = new QStandardItem( time.elapsed() + "ms");
}

void DataPacket::setRawData(QByteArray raw)
{
    QString rawString;
    for(int idx=0; idx<raw.size(); idx++)
    {
            rawString += IHexConverter::hex2string(raw.at(idx)) + " ";
    }
    // Dump last raw packet...
    rawItem = new QStandardItem(rawString);
}

void DataPacket::setAddressData(quint8 address)
{
    addressItem = new QStandardItem(IHexConverter::hex2string(address));
}

void DataPacket::setTypeData(EType currentType)
{
    typeItem = new QStandardItem(currentType);
}

void DataPacket::setContentData(QString content)
{
    contentItem = new QStandardItem(content);
}

void DataPacket::setChecksumData(EChecksum checksum)
{
    checksumItem = new QStandardItem( checksum);

    QBrush brush;
    QColor color;
    if (checksum==Invalid)
    {
        color.setRed(180);
    }
    else
    {
        color.setGreen(180);
    }
    brush.setColor(color);
    brush.setStyle(Qt::SolidPattern);
    checksumItem->setBackground(brush);

    checksumItem->setTextAlignment(Qt::AlignCenter);
}

void DataPacket::updateModel(QStandardItemModel* model)
{
    QList<QStandardItem*> items;

    items.append(timeItem);
    items.append(rawItem);
    items.append(addressItem);
    items.append(typeItem);
    items.append(contentItem);
    items.append(checksumItem);

    model->appendRow(items);
}
