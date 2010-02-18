#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QtGui/QDialog>
#include <QList>
#include <QSettings>

namespace Ui {
    class FindDialog;
}

class QTableView;
class QStandardItem;
class QStandardItemModel;

class FindDialog : public QDialog {
    Q_OBJECT
public:
    FindDialog(QTableView* view, QStandardItemModel* model, QWidget *parent = 0);
    ~FindDialog();

public slots:
    void on_button_close_clicked();
    void on_button_find_clicked();

protected:
    void changeEvent(QEvent *e);

private:
    void highlightItem(QStandardItem* item);

    Ui::FindDialog *m_ui;

    QList<QStandardItem*> foundItems;
    QTableView* logTable;
    QStandardItemModel* model;
    int idx;
    QStandardItem* lastItem;
    QBrush lastBackground;
    QString textToFind;
    QSettings settings;
};

#endif // FINDDIALOG_H
