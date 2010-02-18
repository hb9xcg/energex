#include "fileloader.h"
#include "decoder.h"

#include <QFile>
#include <QMessageBox>

FileLoader::FileLoader(Decoder *decoder) :
        decoder(decoder)
{
    file = NULL;

    QObject::moveToThread(this);
}

bool FileLoader::open(const QString& fileName)
{
    if (file)
    {
        delete file;
    }
    file = new QFile(fileName);
    bool done = file->open(QIODevice::ReadOnly);
    if (!done)
    {
        QString strError("Cannot read file ");
        strError += file->errorString();
//        QMessageBox::warning(this, tr("QTwikeAnalyzer"), strError);
        return false;
    }
    return true;
}

void FileLoader::run()
{
    QDataStream binaryStream( file ); // we will serialize the data into the files
    char character;

    do
    {
        binaryStream.readRawData(&character, 1);
        if (decoder)
        {
            decoder->receiveByte(character);
        }
        else
        {
            emit receiveByte(character);
        }
    }
    while(!binaryStream.atEnd());
}
