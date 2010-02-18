#include "downloader.h"
#include "downloaddialog.h"
#include "ihexconverter.h"
#include "twikeport.h"
#include "checksum.h"


const char Downloader::NOTHING[]= {};
const char Downloader::EXIT[]   = {'x'};
const char Downloader::ESCAPE[] = {0x10,'d'};
const char Downloader::RETURN[] = {'\n','\r'};
const char Downloader::REBOOT[] = {'b','\r'};
const char Downloader::PASSWORD[] = {'A','V','R','U','B'};

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

    connect(port, SIGNAL(receiveData(qint8)),   this, SLOT(receiveData(qint8)), Qt::DirectConnection);
    connect(this, SIGNAL(sendData(QByteArray)), port, SLOT(sendData(QByteArray)),Qt::DirectConnection);
}

void Downloader::receiveData(qint8 data)
{
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

void Downloader::startDownload()
{
    char frameBuffer[FRAME_LENGTH + 5];
    char crcBuffer[FRAME_LENGTH];

    emit appendLog("Rebooting...");

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
        sendCommand(QByteArray(ESCAPE, sizeof(ESCAPE)));
        waitForCommand(1000);
    }


    sendCommand(QByteArray(REBOOT, sizeof(REBOOT)));

    QThread::msleep(200);
    port->close();

    emit appendLog("Connecting Bootloader...");
    port->openBootloader();

    int loginCounter = 0;
    QThread::msleep(200);
    state = eConnect;

    emit sendData(QByteArray(PASSWORD, sizeof(PASSWORD)));

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

        //Sent all data?
        if (frameCounter > (dataBuffer.length() / FRAME_LENGTH))
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
