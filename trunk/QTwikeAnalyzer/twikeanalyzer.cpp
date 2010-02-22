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
#include "twikeanalyzer.h"
#include "settingsdialog.h"
#include "downloaddialog.h"
#include "twikeport.h"
#include <QStringList>
#include <QStandardItemModel>
#include <QSettings>
#include <QByteArray>
#include <QFileDialog>
#include "decoder.h"
#include "fileloader.h"
#include "rawstorage.h"
#include "batteryplot.h"
#include "finddialog.h"
#include "aboutdialog.h"

#include "ui_twikeanalyzer.h"

TwikeAnalyzer::TwikeAnalyzer(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::TwikeAnalyzer)
{
    model = NULL;
    ui->setupUi(this);

    labels << "Time" << "Paket" << "Address" << "Type" << "Message" << "Checksum";

    createNewModel();

    ui->tableLog->setModel(model);

    ui->actionRecord->setEnabled(false);
    ui->actionPlay->setEnabled(false);
    ui->actionPause->setEnabled(false);
    ui->actionStop->setEnabled(false);
    ui->actionDownload->setEnabled(false);

    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));

    // restore current defaults
    readSettings();
    backupTableDimensions();

    if(!currentPort.isEmpty())
    {
        port = new TwikePort(currentPort);
        ui->statusBar->showMessage("Using serial port " + currentPort);
        ui->actionRecord->setEnabled(true);
        ui->actionDownload->setEnabled(true);
    }

    decoder = new Decoder();
    fileLoader = new FileLoader(decoder);
    rawStorage = new RawStorage();

    // Replace battery 1 tab with specialized subclass:
    QWidget* plotParentWidget = ui->qwtPlot1->parentWidget();
    QGridLayout* parentGridLayout = dynamic_cast<QGridLayout*>(plotParentWidget->layout());
    parentGridLayout->removeWidget(ui->qwtPlot1);
    delete ui->qwtPlot1;
    ui->qwtPlot1 = battery1 = new BatteryPlot(plotParentWidget, 1);
    parentGridLayout->addWidget(battery1, 0 ,0 ,1, 1);

    // Replace battery 2 tab with specialized subclass:
    plotParentWidget = ui->qwtPlot2->parentWidget();
    parentGridLayout = dynamic_cast<QGridLayout*>(plotParentWidget->layout());
    parentGridLayout->removeWidget(ui->qwtPlot2);
    delete ui->qwtPlot2;
    ui->qwtPlot2 = battery2 = new BatteryPlot(plotParentWidget, 2);
    parentGridLayout->addWidget(battery2, 0 ,0 ,1, 1);

    // Replace battery 3 tab with specialized subclass:
    plotParentWidget = ui->qwtPlot3->parentWidget();
    parentGridLayout = dynamic_cast<QGridLayout*>(plotParentWidget->layout());
    parentGridLayout->removeWidget(ui->qwtPlot3);
    delete ui->qwtPlot3;
    ui->qwtPlot3 = battery3 = new BatteryPlot(plotParentWidget, 3);
    parentGridLayout->addWidget(battery3, 0 ,0 ,1, 1);


    QIcon windowIcon(QString(":/images/player-time.png"));
    setWindowIcon(windowIcon);
}

TwikeAnalyzer::~TwikeAnalyzer()
{
    delete ui;
    delete rawStorage;
    delete decoder;
}

void TwikeAnalyzer::on_actionSettings_triggered()
{
    SettingsDialog settingsDialog(this);

    if (settingsDialog.exec() != 0)
    {
        if(port && port->isOpen())
        {
            port->close();
        }
        port = new TwikePort(settingsDialog.getPortName());

        if (settingsDialog.getPortName().isEmpty())
        {
            ui->actionRecord->setEnabled(false);
            ui->actionDownload->setEnabled(false);
        }
        else
        {
            currentPort = settingsDialog.getPortName();
            ui->actionRecord->setEnabled(true);
            ui->actionDownload->setEnabled(true);

            QString message("Using serial port ");
            message += currentPort;
            ui->statusBar->showMessage(message);
        }
    }
}

void TwikeAnalyzer::on_actionDownload_triggered()
{
    ui->actionRecord->setEnabled(false);
    ui->actionPlay->setEnabled(false);
    ui->actionPause->setEnabled(false);
    ui->actionStop->setEnabled(false);

    DownloadDialog downloadDialog(this, port);
    downloadDialog.exec();

    ui->actionRecord->setEnabled(true);
}

void TwikeAnalyzer::on_actionOpen_triggered()
{
    QString fileName;
    fileName = QFileDialog::getOpenFileName(this, tr("Open Raw Logfile"), QDir::homePath(), tr("Log Files (*.log *.bin)") );

    if ( fileName.isEmpty())
    {
            return;
    }

    if (fileLoader->open(fileName))
    {
        ui->actionRecord->setEnabled(false);
        ui->actionPlay->setEnabled(false);
        ui->actionPause->setEnabled(true);
        ui->actionStop->setEnabled(false);
        ui->actionDownload->setEnabled(false);

        ui->statusBar->showMessage(fileName);

        battery1->clear();
        battery2->clear();
        battery3->clear();
        decoder->resetTime();

        
        connect(decoder, SIGNAL(receiveMessage(QString, QString, QString, QString, QString, QString)),
                this, SLOT(appendRow(QString, QString, QString, QString, QString, QString)));

        connect(decoder, SIGNAL(battery1(int,float,float)),
                battery1, SLOT(addPower(int,float,float)) );
        connect(decoder, SIGNAL(temperature1(int,int,float)),
                battery1, SLOT(temperature(int,int,float)) );

        connect(decoder, SIGNAL(battery2(int,float,float)),
                battery2, SLOT(addPower(int,float,float)) );
        connect(decoder, SIGNAL(temperature2(int,int,float)),
                battery2, SLOT(temperature(int,int,float)) );

        connect(decoder, SIGNAL(battery3(int,float,float)),
                battery3, SLOT(addPower(int,float,float)) );
        connect(decoder, SIGNAL(temperature3(int,int,float)),
                battery3, SLOT(temperature(int,int,float)) );

        connect(fileLoader, SIGNAL(finished()), this, SLOT(loaderFinished()));

        backupTableDimensions();
        createNewModel();
        fileLoader->start();
    }
}

void TwikeAnalyzer::on_actionSaveAs_triggered()
{
    QString fileName;

    fileName = QFileDialog::getSaveFileName(this, tr("Save Raw Logfile"), QDir::homePath(), tr("Log Files (*.log)"));
    if ( !fileName.isEmpty())
    {
        rawStorage->saveAs(fileName);
    }
}

void TwikeAnalyzer::on_actionRecord_triggered()
{
    backupTableDimensions();
    createNewModel();
    ui->tableLog->setModel(model);
    restoreTableDimensions();
    rawStorage->clear();

    decoder->resetTime();

    connect(decoder, SIGNAL(battery1(int,float,float)),
            battery1, SLOT(addPower(int,float,float)) );
    connect(decoder, SIGNAL(temperature1(int,int,float)),
            battery1, SLOT(temperature(int,int,float)) );

    connect(decoder, SIGNAL(battery2(int,float,float)),
            battery2, SLOT(addPower(int,float,float)) );
    connect(decoder, SIGNAL(temperature2(int,int,float)),
            battery2, SLOT(temperature(int,int,float)) );

    connect(decoder, SIGNAL(battery3(int,float,float)),
            battery3, SLOT(addPower(int,float,float)) );
    connect(decoder, SIGNAL(temperature3(int,int,float)),
            battery3, SLOT(temperature(int,int,float)) );

    connect(port, SIGNAL(receiveData(char)), rawStorage, SLOT(receiveData(char)));
    connect(port, SIGNAL(receiveData(char)), decoder, SLOT(receiveByte(char)));
    connect(decoder, SIGNAL(receiveMessage(QString, QString, QString, QString, QString, QString)),
            this, SLOT(appendRow(QString, QString, QString, QString, QString, QString)));

    port->openTwike();

    ui->actionRecord->setEnabled(false);
    ui->actionPlay->setEnabled(false);
    ui->actionPause->setEnabled(true);
    ui->actionStop->setEnabled(true);
    ui->actionDownload->setEnabled(false);
}

void TwikeAnalyzer::on_actionPause_triggered()
{
    ui->actionPlay->setEnabled(true);
    ui->actionPause->setEnabled(false);
}

void TwikeAnalyzer::on_actionPlay_triggered()
{
    ui->actionPlay->setEnabled(false);
    ui->actionPause->setEnabled(true);
}

void TwikeAnalyzer::on_actionStop_triggered()
{
    rawStorage->disconnect();
    battery1->disconnect();
    battery2->disconnect();
    battery3->disconnect();
    decoder->disconnect();
    disconnect(this, SIGNAL(appendRow(QString, QString, QString, QString, QString, QString)));

    port->close();
    
    ui->actionRecord->setEnabled(true);
    ui->actionPlay->setEnabled(false);
    ui->actionPause->setEnabled(false);
    ui->actionStop->setEnabled(false);
    ui->actionDownload->setEnabled(true);
}

void TwikeAnalyzer::on_actionFind_triggered()
{
    FindDialog findDialog(ui->tableLog, model, this);
    findDialog.exec();
}

void TwikeAnalyzer::on_actionAbout_triggered()
{
    AboutDialog aboutDialog(this);
    aboutDialog.exec();
}

void TwikeAnalyzer::loaderFinished()
{
    battery1->disconnect();
    battery2->disconnect();
    battery3->disconnect();
    decoder->disconnect();

    disconnect(this, SIGNAL(appendRow(QString, QString, QString, QString, QString, QString)));

    ui->tableLog->setModel(model);
    restoreTableDimensions();

    ui->actionRecord->setEnabled(true);
    ui->actionPlay->setEnabled(false);
    ui->actionPause->setEnabled(false);
    ui->actionStop->setEnabled(false);
    ui->actionDownload->setEnabled(true);
}

void TwikeAnalyzer::appendRow(QString time, QString raw, QString address, QString type, QString content, QString checksum)
{
    QList<QStandardItem*> items;

    QStandardItem* checksumItem = new QStandardItem(checksum);
    QBrush brush;
    QColor color;
    if (checksum=="Invalid")
    {
        color.setRed(180);
    }
    else
    {
        color.setGreen(180);
    }
    brush.setColor(color);
    brush.setStyle(Qt::SolidPattern);

    checksumItem->setBackground(brush);
    checksumItem->setTextAlignment(Qt::AlignCenter);

    items.append(new QStandardItem(time));
    items.append(new QStandardItem(raw));
    items.append(new QStandardItem(address));
    items.append(new QStandardItem(type));
    items.append(new QStandardItem(content));
    items.append(checksumItem);

    model->appendRow(items);

    if(!ui->actionPause->isEnabled())
    {
        ui->tableLog->scrollToBottom();
    }
}

void TwikeAnalyzer::readSettings()
{
    QSettings settings;

    settings.beginGroup("TwikeAnalyzer");

    restoreGeometry(settings.value("geometry").toByteArray());

    int idx=0;
    foreach( QString label , labels)
    {
        qint32 width = settings.value(label, "55").toInt();
        ui->tableLog->setColumnWidth(idx++, width );
    }

    currentPort = settings.value("currentPort", "/dev/ttyUSB0").toString();

    settings.endGroup();
}

void TwikeAnalyzer::writeSettings()
{
    QSettings settings;
    settings.beginGroup("TwikeAnalyzer");
    settings.setValue("geometry", saveGeometry());

    int idx=0;
    foreach( QString label, labels)
    {
        qint32 width = ui->tableLog->columnWidth(idx++);
        settings.setValue(label, width );
    }

    settings.setValue("currentPort", currentPort);

    settings.endGroup();
}

void TwikeAnalyzer::closeEvent(QCloseEvent *event)
{
    QSettings settings;
    writeSettings();
    if (port && port->isOpen())
    {
        port->close();
        delete port;
    }
    QWidget::closeEvent(event);
}

void TwikeAnalyzer::createNewModel()
{
    QVector<int> dimensions;

    if (model)
    {
        // Backup dimensions...
        delete model;
    }
    model = new QStandardItemModel(0, labels.size());
    model->setHorizontalHeaderLabels(labels);

}

void TwikeAnalyzer::backupTableDimensions()
{
    // Backup dimensions...
    for( int idx=0; idx < labels.size(); idx++ )
    {
        dimensions.append( ui->tableLog->columnWidth(idx) );
    }
}

void TwikeAnalyzer::restoreTableDimensions()
{
    if (!dimensions.empty())
    {
        for( int idx=0; idx < labels.size(); idx++ )
        {
            ui->tableLog->setColumnWidth(idx, dimensions.at(idx));
        }
    }
}
