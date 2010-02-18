#ifndef RAWSTORAGE_H
#define RAWSTORAGE_H

#include <QObject>

class QFile;
class QDataStream;

class RawStorage : public QObject
{
    Q_OBJECT

    QDataStream* out;
    QFile* file;
    const QString tempFileName;

public:
    RawStorage();
    ~RawStorage();
    void clear();
    void saveAs(const QString& name);

public slots:
    void receiveData(char data);
};


#endif // RAWSTORAGE_H
