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
#include "settingsdialog.h"
#include <QSettings>
#include <qextserialport/qextserialenumerator.h>

#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::SettingsDialog)
{
    m_ui->setupUi(this);

    m_ui->comboSerial->setInsertPolicy(QComboBox::InsertAlphabetically);

    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
    foreach(QextPortInfo port, ports)
    {
        m_ui->comboSerial->addItem(port.portName);
    }

    // restore current default
    QSettings settings;
    settings.beginGroup("Settings");
    QString storedPort = settings.value("Port").toString();
    settings.endGroup();

    if(!storedPort.isEmpty())
    {
        int idx = m_ui->comboSerial->findText(storedPort);
        m_ui->comboSerial->setCurrentIndex(idx);
    }
}

SettingsDialog::~SettingsDialog()
{
    delete m_ui;
}

void SettingsDialog::on_buttonBox_accepted()
{
    // store selection
    QSettings settings;

    settings.beginGroup("Settings");
    settings.setValue("Port", m_ui->comboSerial->currentText());
    settings.endGroup();
}

QString SettingsDialog::getPortName()
{
    return m_ui->comboSerial->currentText();
}

void SettingsDialog::changeEvent(QEvent *e)
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
