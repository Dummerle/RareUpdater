#ifndef RareUpdater_H
#define RareUpdater_H

#include <QDialog>
#include <QFile>
#include <QProcess>
#include <QStandardPaths>
#include <QList>
#include <QPointer>
#include <QSettings>
#include <QDir>
#include <QCheckBox>

#include <QtDebug>

#include "enums.h"
#include "downloader.h"
#include "config.h"

#ifdef QT_DEBUG
#include "console.h"
#define UPDATER_DEBUG(x) x
#else
#define UPDATER_DEBUG(x)
#endif

QT_BEGIN_NAMESPACE
namespace Ui { class RareUpdater; }
QT_END_NAMESPACE

class RareUpdater : public QDialog
{
    Q_OBJECT

public:
    explicit RareUpdater(QWidget *parent = nullptr);
    ~RareUpdater() override;

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
    void current_download_changed(const QString&);

    void logStdOut();
    void logStdErr();
private:
    void processProcess(QStringList cmd);
    QMap<QString, QPointer<QCheckBox>> checkboxes;
    Downloader downloader;
    Config config;
    QSettings settings;
    QProcess* m_proc;
    QProcess* install_process;
    QFile* m_cmdFile;
    QList<QStringList> processes;
    QPointer<QNetworkAccessManager> m_manager;
    Ui::RareUpdater *ui;
    DialogPages init_page;
    QString m_applFolder;
    QString m_tempFolder;
    UPDATER_DEBUG(Console *m_console;)
};
#endif // RareUpdater_H
