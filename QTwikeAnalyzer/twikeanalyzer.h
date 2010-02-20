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
