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
#ifndef IHEXCONVERTER_H
#define IHEXCONVERTER_H

#include <QObject>
#include <QByteArray>

class QFile;

class IHexConverter : public QObject
{
    Q_OBJECT

    QByteArray binBuffer;

    QByteArray convertLine(QString line);

public:
    IHexConverter(QFile* file);
    void open(QFile* file);
    QByteArray getBin();

    static QString hex2string(QByteArray data);
    static QString hex2string(qint8 data);
    static QByteArray string2hex(QString data);
};

#endif // IHEXCONVERTER_H
