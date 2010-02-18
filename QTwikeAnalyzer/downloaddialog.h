#ifndef DOWNLOADDIALOG_H
#define DOWNLOADDIALOG_H

#include <QtGui/QDialog>
#include <QString>
#include <QTime>

namespace Ui {
    class DownloadDialog;
}

class TwikePort;
class Downloader;

class DownloadDialog : public QDialog {
    Q_OBJECT

    void readSettings();
    void writeSettings();
    void openFile(const QString& fileName);

public:
    DownloadDialog(QWidget *parent = 0);
    DownloadDialog(QWidget *parent, TwikePort* port);
    ~DownloadDialog();

signals:
    void closeDownload();
    void startThread();
    void startDownload();


public slots:
    void on_buttonFile_clicked();
    void on_buttonStart_clicked();
    void appendLog(QString text);
    void appendSentData(QByteArray);
    void setProgress(qint32 percentage);
    void appendReceivedData(const qint8 data);

protected:
    void closeEvent(QCloseEvent* event);
    void changeEvent(QEvent *e);

private:
    Ui::DownloadDialog *m_ui;
    Downloader* downloader;
    QString fileName;
    QTime time;
};

#endif // DOWNLOADDIALOG_H
