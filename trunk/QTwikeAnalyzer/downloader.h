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
#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QThread>
#include <QtGlobal>
#include <QByteArray>


class QFile;
class TwikePort;
class DownloadDialog;

class Downloader : public QThread
{
    Q_OBJECT

    void dummy();
    void sendCommand(const QByteArray& command);
    bool waitForCommand(int timeout);

protected:
    virtual void run();

public:
    Downloader(DownloadDialog* dialog, TwikePort* port);

    bool loaded();
    void loadFile(QFile* file);

signals:
    void setProgress(int value);
    void sendData(QByteArray data);
    void sendData(char data[], int size);
    void appendLog(QString text);
    void appendReceivedData(qint8 data);

public slots:
    void receiveData(char data);
    void startDownload();

private:
    TwikePort* port;

    enum EState
    {
        eReboot,
        eCommand,
        eConnect,
        eDownload,
        eInvalid
    };

    EState state;

    enum EProtocol
    {
        eCRC16,
        eSum
    };

    EProtocol protocol;

    qint8 reply;

    static const char NOTHING[];
    static const char EXIT[];
    static const char ESCAPE[];
    static const char RETURN[];
    static const char REBOOT[];
    static const char PASSWORD[];

    static const char XMODEM_NUL;
    static const char XMODEM_SOH;
    static const char XMODEM_STX;
    static const char XMODEM_EOT;
    static const char XMODEM_ACK;
    static const char XMODEM_NAK;
    static const char XMODEM_CAN;
    static const char XMODEM_EOF;
    static const char XMODEM_RWC;

    static const qint32 FRAME_LENGTH;

    QByteArray dataBuffer;
    DownloadDialog* dialog;
};

#endif // DOWNLOADER_H
