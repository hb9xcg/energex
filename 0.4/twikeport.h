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
#ifndef TWIKEPORT_H
#define TWIKEPORT_H

#include <QThread>
#include <QEventLoop>

class QextSerialPort;

class TwikePort : public QThread
{
    Q_OBJECT

    QextSerialPort* port;
    bool running;

protected:
    virtual void run();

public:
    TwikePort(const QString& portName);

    void openTwike();
    void openBootloader();
    void close();
    bool isOpen();
    void flush();

public slots:
    void sendData(char data);
    void sendData(QByteArray data);
    void dummy();

signals:
    void receiveData(char data);
};

#endif // TWIKEPORT_H
