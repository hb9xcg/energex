#include "twikeport.h"
#include <QTimer>
#include <qextserialport/qextserialport.h>

// #include <qextserialenumerator.h> not yet supported!



TwikePort::TwikePort(const QString& portName)
{
#if 0
    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();

    foreach(QextPortInfo port, ports)
    {
        if (port.portName == portName)
        {
            current = port;
            break;
        }
    }
#endif
    port = new QextSerialPort(portName);
    QObject::moveToThread(this);
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

    running = true;
    start();
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

    running = true;
    start();
}

void TwikePort::close()
{
    if (port)
    {
        running = false;
        wait(3000);
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
    int i = port->write( data.constData(), data.length());
    qDebug("sendData : %d", i);
}

void TwikePort::run()
{
    while (running)
    {
        if (port->bytesAvailable() )
        {
            char byte;
            if (port->read(&byte, 1) > 0)
            {
                qint8 data = static_cast<qint8>(byte);
                emit receiveData(data);
            }
        }
        else
        {
            QTimer::singleShot(100, this, SLOT(dummy()));
        }
    }
}

void TwikePort::dummy()
{
}

void TwikePort::flush()
{
    port->flush();
}
