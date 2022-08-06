#include "rareupdater.hpp"
#include "ui_rareupdater.h"

#include <QMessageBox>
#include <QJsonParseError>
#include <QJsonObject>

#include "uninstalldialog.h"
#include "utils.h"

RareUpdater::RareUpdater(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RareUpdater)
{
    ui->setupUi(this);

    UPDATER_DEBUG(
    m_console = new Console(this);
    m_console->show();
    )

    QStringList args = qApp->arguments();
    for (const QString &arg: qAsConst(args)) {
        if (arg == "modify") {
            init_page = DialogPages::SETTINGS;
            break;
        }
    }

    m_data_folder = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    m_temp_folder = QStandardPaths::writableLocation(QStandardPaths::TempLocation);

    m_downloader = new Downloader(m_temp_folder, this);
    connect(m_downloader, &Downloader::progress, this, &RareUpdater::progress_update);
    connect(m_downloader, &Downloader::finished, this, &RareUpdater::download_finished);
    connect(m_downloader, &Downloader::current, this, &RareUpdater::current_download_changed);

    m_proc = new QProcess(this);

    m_cmd_file = new QFile(m_data_folder + "/pythonw.exe");

    ui->extra_space_lbl->setText(ui->extra_space_lbl->text().replace("{}", "0MB"));

    connect(ui->install, &QPushButton::clicked, this, &RareUpdater::install);
    connect(ui->installed_launch, &QPushButton::clicked, this, &RareUpdater::launch);
    connect(ui->uninstall_btn, &QPushButton::clicked, this, &RareUpdater::uninstall);
    connect(ui->update_button, &QPushButton::clicked, this, &RareUpdater::update_rare);
    connect(ui->modify_btn, &QPushButton::clicked, this, &RareUpdater::modify_installation);
    connect(ui->launch_button, &QPushButton::clicked, this, &RareUpdater::launch);
//    // TODO add console

    for (auto &dep: config.opt_dependencies) {
        auto *box = new QCheckBox(dep.name());
        box->setChecked(settings.value(SettingsKeys::get_name_for_dependency(dep.name()), false).toBool());

        auto *info_lbl = new QLabel(dep.info_text());
        ui->optional_group_layout->addRow(box, info_lbl);

        checkboxes[dep.name()] = box;
    }

    m_versions_rare = new Versions("https://pypi.org/pypi/Rare/json", Versions::Component::Rare, this);
    connect(m_versions_rare, &Versions::finished, this, &RareUpdater::updateRareVersions);
    m_versions_rare->fetch();

    m_versions_python = new Versions("https://www.python.org/ftp/python/", Versions::Component::Python, this);
    connect(m_versions_python, &Versions::finished, this, &RareUpdater::updatePythonVersions);
    m_versions_python->fetch();
}

RareUpdater::~RareUpdater() {
    UPDATER_DEBUG(
    delete m_console;
    )
    delete m_downloader;
    delete m_proc;
    delete m_cmd_file;
    delete ui;
}

void RareUpdater::updateRareVersions() {
    qDebug() << "rare: " << m_versions_rare->latest();

    QString current_version(settings.value(SettingsKeys::INSTALLED_VERSION, "").toString());

    ui->version_combo->addItems(m_versions_rare->all());
    ui->version_combo->addItem("git");
    ui->version_combo->setCurrentIndex(1);
    if(current_version != ""){
        ui->version_combo->setCurrentText(current_version);
    }

    if (!settings.contains(SettingsKeys::INSTALLED_VERSION) || init_page == DialogPages::SETTINGS) {
        qDebug() << "Settings";
        ui->page_stack->setCurrentIndex(DialogPages::SETTINGS);
    } else if (current_version == "git"
               || m_versions_rare->latest() == settings.value(SettingsKeys::INSTALLED_VERSION, "")) {
        ui->page_stack->setCurrentIndex(DialogPages::INSTALLED);
        ui->installed_info->setText(current_version);
        ui->space_info->setText(
                Utils::formatSize(Utils::dirSize(m_data_folder))
        );

    } else {
        ui->page_stack->setCurrentIndex(DialogPages::UPDATE);
        ui->update_available_lbl->setText(
                ui->update_available_lbl->text() + " " +
                current_version + " -> " + m_versions_rare->latest());
    }
}

void RareUpdater::updatePythonVersions()
{
    qDebug() << "python: " << m_versions_python->latest();
}

void RareUpdater::download_finished() {

//    QDir appFolder(m_data_folder);
//    if (appFolder.exists())
//        appFolder.removeRecursively();
//    qDebug() << m_data_folder;
//    JlCompress::extractDir(
//            m_temp_folder + "/python-3.10.3-embed-amd64.zip",
//            m_data_folder
//    );
//    QFile pth(m_data_folder + "/python310._pth");
//    if (pth.open(QIODevice::WriteOnly | QIODevice::Append)) {
//        pth.write("import site");
//        pth.close();
//    }

    processProcess(processes.takeFirst());
}

void RareUpdater::modify_installation() {
    ui->page_stack->setCurrentIndex(DialogPages::SETTINGS);
}

void RareUpdater::current_download_changed(const QString &url) {
    ui->status_label->setText("Downloading " + url);
}

void RareUpdater::update_rare() {
    qDebug() << "Update Rare";
    QStringList installCmdRare{
        m_data_folder + "/python.exe", "-m", "pip", "install", "-U",
        "rare"
    };
    ui->version_combo->setCurrentIndex(0);
    ui->update_button->setDisabled(true);
    processProcess(installCmdRare);
}

void RareUpdater::install() {
    QString version(ui->version_combo->currentText());
    ui->install_options_group->setDisabled(true);
    ui->install->setDisabled(true);
    QStringList urls;

    for (auto &dep: config.opt_dependencies) {
        if ((settings.value(SettingsKeys::get_name_for_dependency(dep.name())).toBool(), false)
            && !checkboxes[dep.name()]->isChecked()) {
                processes.append(QStringList{m_data_folder + "/python.exe", "-m", "pip", "uninstall", dep.name()});
                qDebug() << "Will remove " << dep.name();
        }
    }

    QStringList installCmdRare;
    if (version == "git") {
        installCmdRare = QStringList{
            m_data_folder + "/python.exe", "-m", "pip", "install",
            "https://github.com/Dummerle/Rare/archive/refs/heads/main.zip"
        };
    } else {
        installCmdRare = QStringList{
            m_data_folder + "/python.exe", "-m", "pip", "install",
            "rare==" + version
        };
    }

    for (auto &dep: config.opt_dependencies) {
        if (checkboxes[dep.name()]->isChecked()) {
            installCmdRare.append(dep.name());
        }
    }
    qDebug() << installCmdRare;

    if (!settings.contains(SettingsKeys::INSTALLED_VERSION)) {
        urls.append("https://www.python.org/ftp/python/3.10.3/python-3.10.3-embed-amd64.zip");
        urls.append("https://bootstrap.pypa.io/get-pip.py");

        processes.append(QStringList{m_data_folder + "/python.exe", m_temp_folder + "/get-pip.py"});
        processes.append(installCmdRare);

        processes.append(QStringList{
            m_data_folder + "/python.exe", "-m", "pip", "install",
            "https://github.com/Dummerle/legendary/archive/refs/heads/rare.zip"
        });
    }

    if(!QFile(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "\\Rare.lnk").exists()){
        processes.append(QStringList{m_cmd_file->fileName(), "-m", "rare", "--desktop-shortcut"});
    }

    QFile python_exe(m_data_folder + "/python.exe");
    if (python_exe.exists()) {
        processProcess(processes.takeFirst());
    } else {
        m_downloader->downloadFiles(urls);
    }
}

void RareUpdater::processProcess(QStringList cmd) {
    ui->status_label->setText(tr("Running: ") + cmd.join(" "));
    install_process = new QProcess(this);
    install_process->setProcessChannelMode(QProcess::SeparateChannels);
    install_process->setProgram(cmd.takeFirst());
    install_process->setArguments(cmd);

    connect(install_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &RareUpdater::processFinished);
    connect(install_process, &QProcess::readyReadStandardOutput,
            this, &RareUpdater::logStdOut);
    connect(install_process, &QProcess::readyReadStandardError,
            this, &RareUpdater::logStdErr);
    install_process->start();
}

void RareUpdater::logStdOut() {
    UPDATER_DEBUG(
    m_console->readyReadStdout(reinterpret_cast<QProcess*>(sender()));
    )
}

void RareUpdater::logStdErr() {
    UPDATER_DEBUG(
    m_console->readyReadStderr(reinterpret_cast<QProcess*>(sender()));
    )
}


void RareUpdater::processFinished(int exit_code, QProcess::ExitStatus e) {
    if (e == QProcess::ExitStatus::CrashExit) {
        QMessageBox::warning(this, "Error",
                             "Installation failed\n" + QString(install_process->readAllStandardOutput().data()));
        ui->status_label->setText(tr("Installation failed"));
        return;
    }
    install_process->deleteLater();
    if (!processes.isEmpty()) {
        processProcess(processes.takeFirst());
        return;
    }
    settings.setValue(SettingsKeys::INSTALLED_VERSION, ui->version_combo->currentText());
    for (auto &dep: config.opt_dependencies) {
        settings.setValue(SettingsKeys::get_name_for_dependency(dep.name()), checkboxes[dep.name()]->isChecked());
    }

    ui->page_stack->setCurrentIndex(DialogPages::SUCCESS);
}

void RareUpdater::launch() {
    qDebug() << "launch";
    if (m_cmd_file->exists()) {
        m_proc->setProgram(m_cmd_file->fileName());
        QStringList args{ "-m", "rare" };
        m_proc->setArguments(args);
    }
    bool ret = m_proc->startDetached();
    if (ret) qApp->exit();
}

void RareUpdater::cancel() {
    qApp->exit();
}

void RareUpdater::uninstall() {
    UninstallDialog dlg;

    int reply = dlg.uninstall();
    if (reply == UninstallDialog::Reply::Cancel) {
        qDebug() << "Cancel uninstall";
        return;
    }
    QDir app_dir(m_data_folder);
    app_dir.removeRecursively();
    settings.remove(SettingsKeys::INSTALLED_VERSION);
    for (auto &dep: config.opt_dependencies) {
        settings.remove(SettingsKeys::get_name_for_dependency(dep.name()));
    }

    if (reply == UninstallDialog::Reply::Accept) {
        qDebug() << "Remove Files";
        QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).removeRecursively();
        QDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)).removeRecursively();
    }
    QMessageBox::information(this, "Finished", tr("Rare successfully uninstalled"));
    cancel();
}

void RareUpdater::progress_update(int percent) {
    ui->download_progress->setValue(percent);
}
