#ifndef FILELOADER_H
#define FILELOADER_H

#include <QThread>

class QFile;
class Decoder;

class FileLoader : public QThread
{
    Q_OBJECT

protected:
    virtual void run();

public:
    FileLoader(Decoder *decoder=0);
    bool open(const QString& file);

signals:
    void receiveByte(char dataByte);

private:
    QFile* file;
    Decoder* decoder;
};

#endif // FILELOADER_H
