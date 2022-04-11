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
#include <QSettings>
#include <enums.cpp>
#include <JlCompress.h>

#include <QtDebug>

QT_BEGIN_NAMESPACE
namespace Ui { class RareUpdater; }
QT_END_NAMESPACE

class RareUpdater : public QDialog
{
    Q_OBJECT

public:
    RareUpdater(QString init_mode = "", QWidget *parent = nullptr);
    ~RareUpdater();

public slots:
    void downloadError(QNetworkReply::NetworkError err);
    void downloadReadyRead();
    void downloadProgress(qint64 read, qint64 total);
    void downloadFinished(QNetworkReply* reply);
    void installLogs();

private slots:
    void launch(int exit_code, QProcess::ExitStatus e);
    void launch();
    void cancel();
    void install();
    void update_rare();
    void uninstall();
    void modify_installation();
    void processFinished(int exit_code, QProcess::ExitStatus e);

private:
    void processRequest(QNetworkRequest request);
    void processProcess(QString executable);
    bool isHttpRedirect(QNetworkReply *reply);
    void loadingRequestFinished(QNetworkReply *reply);

    DialogPageIndexes pages;
    QSettings settings;
    QString m_applFolder;
    QString m_tempFolder;
    QProcess* m_proc;
    QProcess* install_process;
    QFile* m_cmdFile;
    QList<QNetworkRequest> m_reqList;
    QList<QString> processes;
    QNetworkReply* m_reply;
    QFile* m_downloadFile;
    QPointer<QNetworkAccessManager> m_manager;
    Ui::RareUpdater *ui;
    bool is_init;
    QString init_page;

};
#endif // RareUpdater_H
