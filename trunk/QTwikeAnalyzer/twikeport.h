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
    void receiveData(qint8 data);
};

#endif // TWIKEPORT_H
