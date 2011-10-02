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
#include "twikeport.h"
#include <QTimer>
#include <qextserialport/qextserialport.h>
#include <qextserialport/qextserialenumerator.h>



TwikePort::TwikePort(const QString& portName)
{
    QString current;
    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();

    foreach(QextPortInfo port, ports)
    {
        if (port.portName.contains(portName))
        {
            current = port.physName;
            break;
        }
    }

    port = new QextSerialPort(current);
    connect(port, SIGNAL(readyRead()), this, SLOT(receiveData()));
}

void TwikePort::openTwike()
{
    if(!port)
    {
        qDebug("No serial port specified");
        return;
    }

    if (port->isOpen())
    {
        qDebug("Port not yet closed!");
        close();
    }

    port->setBaudRate(BAUD2400);
    port->setFlowControl(FLOW_OFF);
    port->setParity(PAR_NONE);
    port->setDataBits(DATA_8);
    port->setStopBits(STOP_1);

    port->open(QIODevice::ReadWrite | QIODevice::Unbuffered);
    qDebug("is open: %d", port->isOpen());
}

void TwikePort::openBootloader()
{
    if(!port)
    {
        qDebug("No serial port specified");
        return;
    }

    if (port->isOpen())
    {
        qDebug("Port not yet closed!");
        close();
    }

    port->setBaudRate(BAUD38400);
    port->setFlowControl(FLOW_OFF);
    port->setParity(PAR_NONE);
    port->setDataBits(DATA_8);
    port->setStopBits(STOP_1);

    port->open(QIODevice::ReadWrite | QIODevice::Unbuffered);
    qDebug("is open: %d", port->isOpen());
}

void TwikePort::close()
{
    if (port)
    {
        port->close();
        qDebug("close: is open returned %d", port->isOpen());
    }
}

bool TwikePort::isOpen()
{
    return port->isOpen();
}

void TwikePort::sendData(char data)
{
    int i = port->write( &data, 1);
    qDebug("sendData : %d", i);
}

void TwikePort::sendData(QByteArray data)
{
    port->write( data.constData(), data.length());
}

void TwikePort::receiveData()
{
    if (port->bytesAvailable() )
    {
        char byte;
        if (port->read(&byte, 1) > 0)
        {
            emit receiveData(byte);
        }
    }
}

void TwikePort::flush()
{
    port->flush();
}
