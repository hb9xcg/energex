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
