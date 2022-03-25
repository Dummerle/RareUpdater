#include "backend.h"

Backend::Backend(QObject *parent): QObject{parent}{

    m_manager = new QNetworkAccessManager(this);
}

void Backend::download_python()
{
    qInfo() << ("python install");
}

void Backend::download_python(QString *version)
{
    qInfo() << version;
}
