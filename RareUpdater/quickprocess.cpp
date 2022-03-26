#include "quickprocess.h"


QuickProcess::QuickProcess(QObject *parent):
    QProcess(parent)
{

}

QuickProcess::~QuickProcess()
{

}

void QuickProcess::start(const QString &program, const QVariantList &arguments)
{
    QStringList args;

    for (int i = 0; i < arguments.length(); i++)
        args << arguments[i].toString();
    QProcess::start(program, args);
}

QString QuickProcess::readAll()
{
    return QString(QProcess::readAll());
}
