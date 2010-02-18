#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QString>
#include <QStringList>
#include <QStandardItem>
#include <QVector>


namespace Ui
{
    class TwikeAnalyzer;
}

class QStandardItemModel;
class QCloseEvent;
class TwikePort;
class Decoder;
class FileLoader;
class RawStorage;
class BatteryPlot;

class TwikeAnalyzer : public QMainWindow
{
    Q_OBJECT

    void readSettings();
    void writeSettings();

protected:
    void closeEvent(QCloseEvent *event);

public:
    TwikeAnalyzer(QWidget *parent = 0);
    ~TwikeAnalyzer();

public slots:
    void on_actionSettings_triggered();
    void on_actionDownload_triggered();
    void on_actionOpen_triggered();
    void on_actionSaveAs_triggered();
    void on_actionRecord_triggered();
    void on_actionPause_triggered();
    void on_actionPlay_triggered();
    void on_actionStop_triggered();
    void on_actionFind_triggered();
    void on_actionAbout_triggered();
    void appendRow(QString time, QString raw, QString address, QString type, QString content, QString checksum);
    void loaderFinished();

signals:
    void receiveByte(char character);

private:
    void createNewModel();
    void backupTableDimensions();
    void restoreTableDimensions();

    Ui::TwikeAnalyzer *ui;
    QStandardItemModel* model;
    QString currentPort;
    TwikePort* port;
    Decoder* decoder;
    FileLoader* fileLoader;
    BatteryPlot *battery1, *battery2, *battery3;
    RawStorage* rawStorage;
    QStringList labels;
    QVector<int> dimensions;
};

#endif // MAINWINDOW_H
