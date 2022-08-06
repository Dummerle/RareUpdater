#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QObject>
#include <QStringList>
#include <QPointer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QFile>
#include <JlCompress.h>
#include <QDataStream>
#include <QStandardPaths>


class Downloader : public QObject
{
    Q_OBJECT
public:
    explicit Downloader(const QString&, QObject *parent = nullptr);
    ~Downloader () override;

    void downloadFiles(const QStringList& urls);

signals:
    void progress(int);
    void finished();
    void error(QString, QString);
    void current(QString);

private:
    QPointer<QNetworkAccessManager> m_manager;
    QList<QNetworkRequest> m_requests;
    QNetworkReply *m_reply;
    QFile *m_current_file;
    QString m_data_folder;
    QString m_temp_folder;

    void processRequest(const QNetworkRequest &request);
    bool isHttpRedirect(QNetworkReply *reply);

private slots:
    void downloadError(QNetworkReply::NetworkError err);
    void downloadReadyRead();
    void downloadProgress(qint64 read, qint64 total);
    void downloadFinished(QNetworkReply *);

};


#endif // DOWNLOADER_H
