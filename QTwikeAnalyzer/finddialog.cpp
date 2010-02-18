#include "finddialog.h"
#include "ui_finddialog.h"
#include <QStandardItemModel>
#include <QTableView>



FindDialog::FindDialog(QTableView* view, QStandardItemModel* model, QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::FindDialog),
    logTable(view),
    model(model)
{
    m_ui->setupUi(this);
    idx = 0;
    lastItem = NULL;

    settings.beginGroup("Find");
    QString findTime     = settings.value("Time",     "false").toString();
    QString findPacket   = settings.value("Packet",   "true").toString();
    QString findAddress  = settings.value("Address",  "false").toString();
    QString findType     = settings.value("Type",     "false").toString();
    QString findContent  = settings.value("Content",  "true").toString();
    QString findChecksum = settings.value("Checksum", "false").toString();

    m_ui->editFind->setText( settings.value("Text", "").toString() );
    settings.endGroup();

    m_ui->checkBoxTime->setChecked( findTime == "true" );
    m_ui->checkBoxPaket->setChecked(findPacket == "true");
    m_ui->checkBoxAddress->setChecked(findAddress == "true");
    m_ui->checkBoxType->setChecked(findType == "true");
    m_ui->checkBoxContent->setChecked(findContent == "true");
    m_ui->checkBoxChecksum->setChecked(findChecksum == "true");
}

FindDialog::~FindDialog()
{
    delete m_ui;
}

void FindDialog::changeEvent(QEvent *e)
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

void FindDialog::on_button_close_clicked()
{
    // store selection
    settings.beginGroup("Find");
    settings.setValue("Time"    , m_ui->checkBoxTime->isChecked());
    settings.setValue("Packet"  , m_ui->checkBoxPaket->isChecked());
    settings.setValue("Address" , m_ui->checkBoxAddress->isChecked());
    settings.setValue("Type"    , m_ui->checkBoxType->isChecked());
    settings.setValue("Content" , m_ui->checkBoxContent->isChecked());
    settings.setValue("Checksum", m_ui->checkBoxChecksum->isChecked());

    settings.setValue("Text", m_ui->editFind->text());
    settings.endGroup();

    if (lastItem)
    {
        lastItem->setBackground(lastBackground);
    }
}

void FindDialog::on_button_find_clicked()
{
    if (m_ui->editFind->text() != textToFind )
    {
        textToFind = m_ui->editFind->text();
        Qt::MatchFlags flags;

        flags |= Qt::MatchContains;

        foundItems.clear();

        if(m_ui->checkBoxTime->isChecked())
        {
            foundItems.append( model->findItems(textToFind, flags, 0));
        }
        if(m_ui->checkBoxPaket->isChecked())
        {
            foundItems.append( model->findItems(textToFind, flags, 1));
        }
        if(m_ui->checkBoxAddress->isChecked())
        {
            foundItems.append( model->findItems(textToFind, flags, 2));
        }
        if(m_ui->checkBoxType->isChecked())
        {
            foundItems.append( model->findItems(textToFind, flags, 3));
        }
        if(m_ui->checkBoxContent->isChecked())
        {
            foundItems.append( model->findItems(textToFind, flags, 4));
        }
        if(m_ui->checkBoxChecksum->isChecked())
        {
            foundItems.append( model->findItems(textToFind, flags, 5));
        }
        idx=0;
    }

    if( !foundItems.isEmpty() )
    {
        QStandardItem* item = foundItems.at(idx);
        if( ++idx >= foundItems.size() )
        {
            idx=0;
        }
        logTable->scrollTo(item->index());
        highlightItem(item);
    }
}

void FindDialog::highlightItem(QStandardItem* item)
{
    if (lastItem)
    {
        lastItem->setBackground(lastBackground);
    }
    lastItem = item;
    lastBackground = item->background();
    QBrush brush;
    QColor color;

    color.setRgb(220, 220, 0);

    brush.setColor(color);
    brush.setStyle(Qt::SolidPattern);
    item->setBackground(brush);
}
