#include "settingsdialog.h"
#include <QSettings>

#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::SettingsDialog)
{
    m_ui->setupUi(this);

    m_ui->comboSerial->setInsertPolicy(QComboBox::InsertAlphabetically);

    m_ui->comboSerial->addItem("/dev/ttyUSB0");

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
