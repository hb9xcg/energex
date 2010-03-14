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
