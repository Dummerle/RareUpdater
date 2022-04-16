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
#include <QDir>
#include "downloader.h"
#include "config.cpp"
#include <QtDebug>
#include <QCheckBox>

QT_BEGIN_NAMESPACE
namespace Ui { class RareUpdater; }
QT_END_NAMESPACE

class RareUpdater : public QDialog
{
    Q_OBJECT

public:
    explicit RareUpdater(QString init_mode = "", QWidget *parent = nullptr);
    ~RareUpdater() override;

public slots:

    void installLogs();

private slots:
    void launch();
    void cancel();
    void install();
    void update_rare();
    void uninstall();
    void modify_installation();
    void progress_update(int);
    void download_finished();
    void processFinished(int exit_code, QProcess::ExitStatus e);
    void loadingRequestFinished(QNetworkReply *reply);

private:
    void processProcess(const QString& executable);
    QMap<QString, QPointer<QCheckBox>> checkboxes;
    Downloader downloader;
    DialogPageIndexes pages;
    Config cfg;
    QSettings settings;
    QProcess* m_proc;
    QProcess* install_process{};
    QFile* m_cmdFile;
    QList<QString> processes;
    QPointer<QNetworkAccessManager> m_manager;
    Ui::RareUpdater *ui;
    QString init_page;
    QString m_applFolder;
    QString m_tempFolder;
};
#endif // RareUpdater_H
