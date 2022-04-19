#ifndef CONSOLE_H
#define CONSOLE_H

#include <QDialog>
#include <QProcess>

namespace Ui {
class Console;
}

class Console : public QDialog
{
    Q_OBJECT

public:
    explicit Console(QWidget *parent = nullptr);
    ~Console();

public slots:
    void readyReadStdout(QProcess* proc);
    void readyReadStderr(QProcess* proc);

private:
    QProcess* proc;

    Ui::Console *ui;
};

#endif // CONSOLE_H
