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
#include "versions.hpp"
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
    void current_download_changed(const QString&);

    void logStdOut();
    void logStdErr();

private:
    void updateRareVersions();
    void updatePythonVersions();

    void processProcess(QStringList cmd);

    QFile* m_cmd_file;

    Versions *m_versions_rare;
    Versions *m_versions_python;

    QString m_data_folder;
    QString m_temp_folder;

    Downloader *m_downloader;

    QProcess* m_proc;
    QProcess* install_process;

    QMap<QString, QPointer<QCheckBox>> checkboxes;
    Config config;
    QSettings settings;

    QList<QStringList> processes;
    DialogPages init_page;

    UPDATER_DEBUG(Console *m_console;)
    Ui::RareUpdater *ui;
};
#endif // RareUpdater_H
