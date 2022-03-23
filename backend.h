#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QNetworkAccessManager>

class Backend : public QObject
{
    Q_OBJECT
public:
    explicit Backend(QObject *parent = nullptr);
    void download_python(QString *version);

private:
    QNetworkAccessManager manager();

signals:

private slots:

};

#endif // BACKEND_H
