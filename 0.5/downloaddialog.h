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

public slots:
    void on_buttonFile_clicked();
    void on_buttonStartMediator_clicked();
    void on_buttonStartPlc_clicked();
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
