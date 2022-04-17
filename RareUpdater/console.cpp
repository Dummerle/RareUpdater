#include "console.h"
#include "ui_console.h"

Console::Console(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Console)
{
    ui->setupUi(this);
    QFont font = this->font();
    font.setFamily("monospace");
    font.setStyleHint(QFont::TypeWriter);
    ui->console->setFont(font);

    ui->console->appendPlainText("RareUpdater debugging console");
}

Console::~Console()
{
    delete ui;
}

void Console::readyReadStdout(QProcess* proc)
{
    QString text(proc->readAllStandardOutput());
    QString html("<p style=\"white-space:pre\">%1</p>");
    ui->console->appendHtml(html.arg(text));
}

void Console::readyReadStderr(QProcess* proc)
{
    QString text(proc->readAllStandardError());
    QString html("<p style=\"color:red;white-space:pre\">%1</p>");
    ui->console->appendHtml(html.arg(text));
}
