#ifndef UNINSTALLDIALOG_H
#define UNINSTALLDIALOG_H

#include <QDialog>

namespace Ui {
class UninstallDialog;
}

class UninstallDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UninstallDialog(QWidget *parent = nullptr);
    ~UninstallDialog();
    int uninstall();

private slots:
    void ok_clicked();
    void cancel_clicked();

private:
    Ui::UninstallDialog *ui;
    bool accepted = false;
};

#endif // UNINSTALLDIALOG_H
