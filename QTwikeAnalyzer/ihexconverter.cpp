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
#include "ihexconverter.h"
#include <QFile>
#include <QTextStream>
#include <QString>

IHexConverter::IHexConverter(QFile* file)
{
    open(file);
}

void IHexConverter::open(QFile* file)
{
    QTextStream textStream(file);

    QString line = textStream.readLine();
    while (!line.isEmpty() && line != ":00000001FF")
    {
        QByteArray lineBuffer = convertLine(line);
        binBuffer.append(lineBuffer);
        line = textStream.readLine();
    }
}

QByteArray IHexConverter::convertLine(QString line)
{
    QString dataHex = line.mid(9, line.length()-2-9);
    return string2hex(dataHex);;
}

QByteArray IHexConverter::getBin()
{
    return binBuffer;
}

QString IHexConverter::hex2string(QByteArray data)
{
    QString str;
    QString hexNbr;
    for (int i=0; i<data.length(); i++)
    {
        hexNbr = QString("%1").arg(data[i], 2, 16, QLatin1Char('0'));
        hexNbr = hexNbr.toUpper();
        hexNbr = hexNbr.right(2);
        hexNbr += " ";
        str += hexNbr;
    }
    return str;
}

QString IHexConverter::hex2string(qint8 data)
{
    QString hexNbr;

    hexNbr = QString("%1").arg(data, 2, 16, QLatin1Char('0'));
    hexNbr = hexNbr.toUpper();
    if (hexNbr.length() == 1)
    {
        hexNbr = "0" + hexNbr;
    }
    else if (hexNbr.length() > 2)
    {
        hexNbr = hexNbr.mid(6, 2);
    }
    return hexNbr;
}

QByteArray IHexConverter::string2hex(QString data)
{
    QByteArray buffer;// (data.length()/2);
    int j = 0;
    for (int i=0; i<data.length(); i+=2)
    {
        QString str;
        str += data.at(i);
        str += data.at(i+1);
        bool ok;
        int number = str.toInt(&ok, 16);
        buffer[j] = (qint8) (number & 0xFF);
        j++;
    }
    return buffer;
}
