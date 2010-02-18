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
