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
#include "downloader.h"
#include "downloaddialog.h"
#include "ihexconverter.h"
#include "twikeport.h"
#include "checksum.h"


const char Downloader::NOTHING[]= {};
const char Downloader::EXIT[]   = {'x'};
const char Downloader::ESCAPE_MEDIATOR[] = {0x10,'d'};
const char Downloader::ESCAPE_PLC[]      = {0x10,'p'};
const char Downloader::RETURN[] = {'\n','\r'};
const char Downloader::REBOOT[] = {'b','\r'};
const char Downloader::PASSWORD_MEDIATOR[] = {'A','V','R','U','B'};
const char Downloader::PASSWORD_PLC[] = {'A','V','R','P','L'};

const char Downloader::XMODEM_NUL = 0x00;
const char Downloader::XMODEM_SOH = 0x01;
const char Downloader::XMODEM_STX = 0x02;
const char Downloader::XMODEM_EOT = 0x04;
const char Downloader::XMODEM_ACK = 0x06;
const char Downloader::XMODEM_NAK = 0x15;
const char Downloader::XMODEM_CAN = 0x18;
const char Downloader::XMODEM_EOF = 0x1A;
const char Downloader::XMODEM_RWC = 0x43; //'C'

const qint32 Downloader::FRAME_LENGTH = 128;


Downloader::Downloader(DownloadDialog* dialog, TwikePort* port)
{
    this->dialog = dialog;
    this->port = port;
    state = eInvalid;
    protocol = eCRC16;

    QObject::moveToThread(this);

    connect(port, SIGNAL(receiveData(char)),   this, SLOT(receiveData(char)), Qt::DirectConnection);
    connect(this, SIGNAL(sendData(QByteArray)), port, SLOT(sendData(QByteArray)),Qt::DirectConnection);
}

void Downloader::receiveData(char byte)
{
    char data = static_cast<char>(byte);
    emit appendReceivedData(data);

    switch(state)
    {
    case eReboot:
        if (data == '>')
        {
            state = eCommand;
        }
        break;

    case eConnect:
        if (data == 'C')
        {
            state = eDownload;
        }
        break;

    case eDownload:
        if (data == XMODEM_ACK)
        {
            reply = XMODEM_ACK;
        }
        else
        {
            reply = XMODEM_NAK;
        }
        break;

    default:
        break;
    }
}

void Downloader::loadFile(QFile* file)
{
    IHexConverter data(file);
    dataBuffer = data.getBin();
}

bool Downloader::loaded()
{
    return !dataBuffer.isEmpty();
}

void Downloader::startPlc()
{
    rebootPlc();
    connectPlc();
    download();
}

void Downloader::startMediator()
{
    rebootMediator();
    connectMediator();
    download();
}

void Downloader::rebootPlc()
{
    emit appendLog("Rebooting PLC...");

    emit setProgress(0);

    if (port->isOpen())
    {
            port->close();
    }
    port->openTwike();

    state = eReboot;

    sendCommand(QByteArray(NOTHING, sizeof(NOTHING)));

    if (waitForCommand(500) == false)
    {
        sendCommand(QByteArray(ESCAPE_PLC, sizeof(ESCAPE_PLC)));
        waitForCommand(1000);
    }

    sendCommand(QByteArray(REBOOT, sizeof(REBOOT)));

    QThread::msleep(200);
    port->close();
}

void Downloader::rebootMediator()
{
    emit appendLog("Rebooting Mediator...");

    emit setProgress(0);

    if (port->isOpen())
    {
            port->close();
    }
    port->openTwike();

    state = eReboot;

    sendCommand(QByteArray(NOTHING, sizeof(NOTHING)));

    if (waitForCommand(500) == false)
    {
        sendCommand(QByteArray(ESCAPE_MEDIATOR, sizeof(ESCAPE_MEDIATOR)));
        waitForCommand(1000);
    }

    sendCommand(QByteArray(REBOOT, sizeof(REBOOT)));

    QThread::msleep(200);
    port->close();
}

void Downloader::connectPlc()
{
    emit appendLog("Connecting PLC Bootloader...");
    port->openBootloader();

    int loginCounter = 0;
    QThread::msleep(200);
    state = eConnect;

    emit sendData(QByteArray(PASSWORD_PLC, sizeof(PASSWORD_PLC)));

    do
    {
        QThread::msleep(100);
        if (++loginCounter>15)
        {
            emit appendLog("Login failed.");
            return;
        }
    }
    while (state != eDownload);

    emit appendLog("Connected!");
}

void Downloader::connectMediator()
{
    emit appendLog("Connecting Mediator Bootloader...");
    port->openBootloader();

    int loginCounter = 0;
    QThread::msleep(200);
    state = eConnect;

    emit sendData(QByteArray(PASSWORD_MEDIATOR, sizeof(PASSWORD_MEDIATOR)));

    do
    {
        QThread::msleep(100);
        if (++loginCounter>15)
        {
            emit appendLog("Login failed.");
            return;
        }
    }
    while (state != eDownload);

    emit appendLog("Connected!");
}

void Downloader::download()
{
    char frameBuffer[FRAME_LENGTH + 5];
    char crcBuffer[FRAME_LENGTH];
    int frameCounter = 0;
    int retransmissionCount = 0;
    qint8 retransmissionNbr = 0;
    qint8 PackNo = 1;
    int crc = 0;

    do
    {
        reply = XMODEM_NUL;

        //Package No
        frameBuffer[0] = XMODEM_SOH;
        frameBuffer[1] = PackNo;
        frameBuffer[2] = (qint8) (0xFF ^ (int)PackNo);

        //Data
        for (int i=0; i<FRAME_LENGTH; i++)
        {
            char dataByte = -1;
            int idx = frameCounter * FRAME_LENGTH + i;
            if (idx<dataBuffer.length()) {
                    dataByte = dataBuffer[idx];
            }
            frameBuffer[3+i] = dataByte;
            crcBuffer[i] = dataByte;
        }

        //Checksum
        switch(protocol)
        {
        case eCRC16:
            crc = Checksum::crc16(crcBuffer, FRAME_LENGTH, 0x1021, (short)0);
            break;
        case eSum:
            crc = Checksum::sumChecksum(crcBuffer, FRAME_LENGTH, 0x1021, (short)0);
            break;
        default:
            crc = 0;
        }

        frameBuffer[FRAME_LENGTH + 3] = (qint8) (crc / 256);
        frameBuffer[FRAME_LENGTH + 4] = (qint8) (crc % 256);

        emit sendData(QByteArray(frameBuffer, sizeof(frameBuffer)));

        emit setProgress( (frameCounter * 100) / (dataBuffer.length() / FRAME_LENGTH) );

        PackNo++;

        // wait for respond
        bool waiting = true;
        for(int t=0; t<500 && waiting; t++)
        {
            QThread::msleep(1);
            switch(reply) {
            case XMODEM_NAK: //request resend
            retransmissionCount++;
            retransmissionNbr++;
            waiting = false;
            break;
            case XMODEM_ACK: //send success, next
            retransmissionCount = 0;
            frameCounter++;
            waiting = false;
            break;
        default:
            break;
            }
        }

        //timeout
        if (reply == XMODEM_NUL)
        {
            retransmissionCount++;
            retransmissionNbr++;
        }

        if (retransmissionCount > 3 || retransmissionNbr > 10)
        {
            emit appendLog("Number of retries exceeded.");
            break;
        }

        // Sent all data?
        // Align to 256bytes in order to match ATMega644p page size.
        if (frameCounter/2 > (dataBuffer.length() / FRAME_LENGTH)/2)
        {
            //send finish
            char endBuffer[1];
            endBuffer[0] = XMODEM_EOT;
            emit sendData(QByteArray(endBuffer, sizeof(endBuffer)));
            emit appendLog("Download successful.");
            emit setProgress(100);
            break;
        }
    }
    while(true);
}

void Downloader::sendCommand(const QByteArray& command)
{
    QByteArray buffer;
    for (int i=0; i<command.size(); i++)
    {
        buffer.clear();
        buffer.append(command.at(i));
        emit sendData(buffer);
        port->flush();
        QThread::msleep(20);
    }

    for (unsigned int i=0; i<sizeof(RETURN); i++)
    {
        buffer.clear();
        buffer.append(RETURN[i]);
        emit sendData(buffer);
        port->flush();
        QThread::msleep(20);
    }
}

bool Downloader::waitForCommand(int timeout)
{
    for (int waitCylces=0; waitCylces<10; waitCylces++)
    {
        QThread::msleep(timeout/10); // Wait for echo
        if (state == eCommand)
        {
            QThread::msleep(50); // Wait for echo
            return true;
        }
    }
    return false;
}

void Downloader::run()
{
    exec();
}

void Downloader::dummy()
{
}

/*
void Downloader::sendData(byte[] data)
{
        port.sendData(data);
        String text = HexConverter.hex2string(data);
        sendData.emit(text);
}

void Downloader::run()
{
    eventLoop = new QEventLoop();
    eventLoop.exec();

    port.close();

    port = NULL;
}


void Downloader::close()
{
    eventLoop.quit();
}
*/
