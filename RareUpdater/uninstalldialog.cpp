#include "uninstalldialog.h"
#include "ui_uninstalldialog.h"

UninstallDialog::UninstallDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UninstallDialog){

    ui->setupUi(this);

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(ok_clicked()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(cancel_clicked()));
}

UninstallDialog::~UninstallDialog() {
    delete ui;
}

void UninstallDialog::ok_clicked(){
    accepted = true;
    this->close();
}
void UninstallDialog::cancel_clicked(){
    this->close();
}

int UninstallDialog::uninstall(){
    this->exec();
    if(accepted){
        if(ui->keep_check->isChecked()){
            return 2;
        }
        else{
            return 1;
        }
    }
    else{
        return 0;
    }
}
