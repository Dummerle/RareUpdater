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


class Downloader : public QObject {
Q_OBJECT
public:
    explicit Downloader(QObject *parent = nullptr);

    void download_files(const QStringList& urls);

private:
    void process_request(const QNetworkRequest &request);

    QNetworkReply *m_reply{};
    QFile *m_downloadFile{};
    QPointer<QNetworkAccessManager> m_manager;
    QList<QNetworkRequest> m_reqList;
    QString m_applFolder;
    QString m_tempFolder;

    bool isHttpRedirect(QNetworkReply *reply);

signals:
    void progress_update(int);
    void finished();
    void current_download_changed(QString);

private slots:
    void downloadError(QNetworkReply::NetworkError err);
    void downloadReadyRead();
    void downloadProgress(qint64 read, qint64 total);
    void downloadFinished(QNetworkReply *);

};


#endif // DOWNLOADER_H
