#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QtGui/QDialog>

namespace Ui {
    class SettingsDialog;
}

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();

    QString getPortName();

public slots:
    void on_buttonBox_accepted();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::SettingsDialog *m_ui;
};

#endif // SETTINGSDIALOG_H
