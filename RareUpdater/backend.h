#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QtDebug>

class Backend : public QObject
{
    Q_OBJECT
public:
    explicit Backend(QObject *parent = nullptr);

public slots:
    void download_python();
    void download_python(QString *version);

private:
    QNetworkAccessManager* m_manager;

private slots:

};

#endif // BACKEND_H
