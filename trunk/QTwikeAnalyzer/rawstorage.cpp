#include <QFile>
#include <QDir>
#include <QDataStream>
#include "rawstorage.h"

RawStorage::RawStorage()
 : tempFileName(QDir::tempPath() + "/QTwikeAnalyzer.tmp")
{
    file = new QFile(tempFileName);
    file->open(QIODevice::Truncate|QIODevice::WriteOnly);
    out = new QDataStream(file);
}

RawStorage::~RawStorage()
{
    delete out;
    delete file;
    QFile::remove(tempFileName);
}

void RawStorage::saveAs(const QString& name)
{
    file->close();

    QFile::rename(tempFileName, name);
    file->open(QIODevice::Truncate|QIODevice::WriteOnly);

    out->setDevice(file);
}

void RawStorage::clear()
{
    file->close();
    file->open(QIODevice::Truncate|QIODevice::WriteOnly);
    out = new QDataStream(file);
}

void RawStorage::receiveData(char data)
{
    out->writeRawData(&data, 1);
}
