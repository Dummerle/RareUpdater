#ifndef RareUpdater_H
#define RareUpdater_H

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
namespace Ui { class RareUpdater; }
QT_END_NAMESPACE

class RareUpdater : public QDialog
{
    Q_OBJECT

public:
    RareUpdater(QWidget *parent = nullptr);
    ~RareUpdater();

public slots:
    void downloadError(QNetworkReply::NetworkError err);
    void downloadReadyRead();
    void downloadProgress(qint64 read, qint64 total);
    void downloadFinished(QNetworkReply* reply);

private slots:
    void launch(int exit_code, QProcess::ExitStatus e);
    void launch();
    void cancel();
    void install();
    void installPip();
    void installRare(int exit_code, QProcess::ExitStatus e);

private:
    void processRequest(QNetworkRequest request);
    bool isHttpRedirect(QNetworkReply *reply);

    QString m_applFolder;
    QString m_tempFolder;
    QProcess* m_proc;
    QProcess* install_process;
    QFile* m_cmdFile;
    QList<QNetworkRequest> m_reqList;
    QNetworkReply* m_reply;
    QFile* m_downloadFile;
    QPointer<QNetworkAccessManager> m_manager;
    Ui::RareUpdater *ui;
    QProcess* pip_process;
    QProcess* get_pip_process;

};
#endif // RareUpdater_H
