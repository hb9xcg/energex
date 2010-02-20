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
#include "downloaddialog.h"
#include "downloader.h"
#include "ihexconverter.h"
#include <QFile>
#include <QMessageBox>
#include <QSettings>
#include <QFileDialog>

#include "ui_downloaddialog.h"

DownloadDialog::DownloadDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::DownloadDialog)
{
    m_ui->setupUi(this);
}

DownloadDialog::DownloadDialog(QWidget* parent, TwikePort* port) :
    QDialog(parent),
    m_ui(new Ui::DownloadDialog)
{
    m_ui->setupUi(this);
    m_ui->progressBar->setMaximum(100);
    m_ui->progressBar->setValue(0);

    downloader = new Downloader(this, port);
    connect(this, SIGNAL(startThread()), downloader, SLOT(start()));
    emit startThread();

    connect(downloader, SIGNAL(appendReceivedData(qint8)), this, SLOT(appendReceivedData(qint8)));
    connect(downloader, SIGNAL(sendData(QByteArray)), this, SLOT(appendSentData(QByteArray)));

    connect(downloader, SIGNAL(setProgress(int)), m_ui->progressBar, SLOT(setValue(int)));
    connect(this, SIGNAL(startDownload()), downloader, SLOT(startDownload()));
    connect(downloader, SIGNAL(appendLog(QString)), this, SLOT(appendLog(QString)));

    connect(this, SIGNAL(closeDownload()), downloader, SLOT(quit()));

    downloader->start();

    readSettings();

    openFile(fileName);

    m_ui->buttonStart->setEnabled( downloader->loaded() );
}

DownloadDialog::~DownloadDialog()
{
    delete m_ui;
}

void DownloadDialog::openFile(const QString& fileName)
{
    if (fileName.isEmpty())
    {
        return;
    }

    QFile file(fileName);
    bool done = file.open(QIODevice::ReadOnly);
    if (!done)
    {
        QString strError("Cannot read file ");
        strError += file.errorString();
        QMessageBox::warning(this, tr("Application"), strError);
        return;
    }

    m_ui->projectFile->setText(fileName);
    downloader->loadFile(&file);
}

void DownloadDialog::on_buttonFile_clicked()
{
    QString filter(tr("Intel Hex Files (*.hex *.ihex)"));
    fileName = QFileDialog::getOpenFileName(this, tr("Open Raw Logfile"), QDir::homePath(), filter );

    openFile(fileName);
    m_ui->buttonStart->setEnabled( downloader->loaded() );
}

void DownloadDialog::on_buttonStart_clicked()
{
    time.start();
    emit startDownload();
}

void DownloadDialog::appendLog(QString text)
{
    QString currentTime = QString("%1").arg(time.elapsed(), 5, 10, QLatin1Char(' '));
    text = currentTime + "ms:\t" + text;
    m_ui->textLog->appendPlainText(text);
}

void DownloadDialog::appendSentData(QByteArray data)
{
    QString line = IHexConverter::hex2string(data);
    line = "send: " + line;
    int fill = 24 - line.length();
    if (fill > 8)
    {
        while (fill-->0)
        {
            line += ' ';
        }
        for (int i=0; i<data.size(); i++)
        {
            if ( isprint(data.at(i)) )
            {
                line += QString("%1").arg(data.at(i));
            }
            else
            {
                line += '.';
            }
        }
    }
    appendLog(line);
}

void DownloadDialog::setProgress(qint32 percentage)
{
    m_ui->progressBar->setValue(percentage);
}


void DownloadDialog::appendReceivedData(const qint8 data)
{
    QString line = QString("%1").arg(data, 2, 16, QLatin1Char('0'));
    line = line.toUpper();
    line = line.right(2);
    line = "recv: " + line;
    int fill = 24 - line.length();
    if (fill > 8)
    {
        while (fill-->0)
        {
            line += ' ';
        }
        if ( isprint(data) )
        {
            line += QString("%1").arg(char(data));
        }
        else
        {
            line += '.';
        }
    }
    appendLog(line);
}

void DownloadDialog::readSettings()
{
    QSettings settings;

    settings.beginGroup("DownloadDialog");
    restoreGeometry(settings.value("geometry").toByteArray());

    fileName = settings.value("fileName", "").toString();
    settings.endGroup();
}

void DownloadDialog::writeSettings()
{
    QSettings settings;
    settings.beginGroup("DownloadDialog");
    settings.setValue("geometry", saveGeometry());

    settings.setValue("fileName", fileName);

    settings.endGroup();
}

void DownloadDialog::closeEvent(QCloseEvent* event)
{
    emit closeDownload();
    writeSettings();
    downloader->wait(3000);
    QWidget::closeEvent(event);
}

void DownloadDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
