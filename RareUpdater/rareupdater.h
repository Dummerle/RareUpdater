#ifndef RARELAUNCHER_H
#define RARELAUNCHER_H

#include <QDialog>
#include <QFile>
#include <QProcess>
#include <QStandardPaths>
#include <QList>
#include <QPointer>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include <JlCompress.h>

#include <QtDebug>

QT_BEGIN_NAMESPACE
namespace Ui { class RareLauncher; }
QT_END_NAMESPACE

class RareLauncher : public QDialog
{
    Q_OBJECT

public:
    RareLauncher(QWidget *parent = nullptr);
    ~RareLauncher();

public slots:
    void downloadError(QNetworkReply::NetworkError err);
    void downloadReadyRead();
    void downloadProgress(qint64 read, qint64 total);
    void downloadFinished(QNetworkReply* reply);

private slots:
    void launch();
    void cancel();
    void install();

private:
    void processRequest(QNetworkRequest request);
    bool isHttpRedirect(QNetworkReply *reply);

    QString m_applFolder;
    QString m_tempFolder;
    QProcess* m_proc;
    QFile* m_cmdFile;
    QList<QNetworkRequest> m_reqList;
    QNetworkReply* m_reply;
    QFile* m_downloadFile;
    QPointer<QNetworkAccessManager> m_manager;
    Ui::RareLauncher *ui;
};
#endif // RARELAUNCHER_H
