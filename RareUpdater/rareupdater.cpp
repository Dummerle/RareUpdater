#include "rareupdater.h"
#include "ui_rareupdater.h"
#include <QMessageBox>
#include <QJsonParseError>
#include <QJsonObject>
#include <utility>
#include <QDialogButtonBox>
#include "uninstalldialog.h"
#include "utils.h"


RareUpdater::RareUpdater(QString init, QWidget *parent)
        : QDialog(parent), ui(new Ui::RareUpdater) {
    ui->setupUi(this);
    init_page = std::move(init);
    m_proc = new QProcess(this);

    m_applFolder = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    m_tempFolder = QStandardPaths::writableLocation(QStandardPaths::TempLocation);

    m_cmdFile = new QFile(m_applFolder + "\\pythonw.exe");

    ui->extra_space_lbl->setText(ui->extra_space_lbl->text().replace("{}", "0MB"));

    connect(ui->install, SIGNAL(clicked()), this, SLOT(install()));
    connect(ui->installed_launch, SIGNAL(clicked()), this, SLOT(launch()));
    connect(ui->uninstall_btn, SIGNAL(clicked()), this, SLOT(uninstall()));
    connect(ui->update_button, SIGNAL(clicked()), this, SLOT(update_rare()));
    connect(ui->modify_btn, SIGNAL(clicked()), this, SLOT(modify_installation()));
    connect(ui->launch_button, SIGNAL(clicked()), this, SLOT(launch()));
    // TODO add console
    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, SIGNAL(finished(QNetworkReply * )), this, SLOT(loadingRequestFinished(QNetworkReply * )));
    m_manager->get(QNetworkRequest(QUrl("https://pypi.org/pypi/Rare/json")));

    connect(&downloader, SIGNAL(progress_update(int)), this, SLOT(progress_update(int)));
    connect(&downloader, SIGNAL(finished()), this, SLOT(download_finished()));
    connect(&downloader, SIGNAL(current_download_changed(QString)), this,
            SLOT(current_download_changed(const QString &)));

    for (auto &dep: cfg.opt_dependencies) {
        auto *box = new QCheckBox(dep.getName());
        box->setChecked(settings.value(SettingsKeys::get_name_for_dependency(dep.getName()), false).toBool());

        auto *info_lbl = new QLabel(dep.getInfo());
        ui->optional_group_layout->addRow(box, info_lbl);

        checkboxes[dep.getName()] = box;

    }
}

RareUpdater::~RareUpdater() {
    delete ui;
}

void RareUpdater::loadingRequestFinished(QNetworkReply *reply) {
    if (reply->error() != QNetworkReply::NoError) {
        QMessageBox::warning(this, "Error", reply->errorString());
        return;
    }
    QJsonParseError error{};
    QJsonDocument json(QJsonDocument::fromJson(reply->readAll().data(), &error));
    QStringList releases;
    releases = json.object()["releases"].toObject().keys();
    std::reverse(releases.begin(), releases.end());
    releases.insert(0, "git");
    QString current_version(settings.value(SettingsKeys::INSTALLED_VERSION, "").toString());

    ui->version_combo->addItems(releases);
    ui->version_combo->setCurrentIndex(1);
    reply->close();
    reply->deleteLater();
    if(current_version != ""){
        ui->version_combo->setCurrentText(current_version);
    }

    if (!settings.contains(SettingsKeys::INSTALLED_VERSION) || init_page == "modify") {
        qDebug() << "Settings";
        ui->page_stack->setCurrentIndex(pages.SETTINGS);
    } else if (current_version == "git"
               || releases[1] == settings.value(SettingsKeys::INSTALLED_VERSION, "")) {
        ui->page_stack->setCurrentIndex(pages.INSTALLED);
        ui->installed_info->setText(current_version);
        ui->space_info->setText(
                Utils::formatSize(Utils::dirSize(m_applFolder))
        );

    } else {
        ui->page_stack->setCurrentIndex(pages.UPDATE);
        ui->update_available_lbl->setText(
                ui->update_available_lbl->text() + " " +
                current_version + " -> " + releases[1]);
    }
}

void RareUpdater::download_finished() {
    processProcess(processes.takeFirst());
}

void RareUpdater::modify_installation() {
    ui->page_stack->setCurrentIndex(pages.SETTINGS);
}

void RareUpdater::current_download_changed(const QString &url) {
    ui->status_label->setText("Downloading " + url);
}

void RareUpdater::update_rare() {
    qDebug() << "Update Rare";
    QString pipInstallCmd(m_applFolder + "\\python.exe -m pip install -U rare");
    ui->version_combo->setCurrentIndex(0);
    ui->update_button->setDisabled(true);
    processProcess(pipInstallCmd);
}

void RareUpdater::install() {
    QString version(ui->version_combo->currentText());
    ui->install_options_group->setDisabled(true);
    ui->install->setDisabled(true);
    QStringList urls;

    for (auto &dep: cfg.opt_dependencies) {
        if ((settings.value(SettingsKeys::get_name_for_dependency(dep.getName())).toBool(), false)
            && !checkboxes[dep.getName()]->isChecked()) {
            processes.append(m_applFolder + "\\python.exe -m pip uninstall " + dep.getName());
            qDebug() << "Will remove " << dep.getName();
        }
    }
    QString pipInstallCmd;
    if (version == "git") {
        pipInstallCmd =
                m_applFolder + "python.exe -m pip install https://github.com/Dummerle/Rare/archive/refs/heads/main.zip";
    } else {
        pipInstallCmd = m_applFolder + "\\python.exe -m pip install rare==" + version;
    }
    for (auto &dep: cfg.opt_dependencies) {
        if (checkboxes[dep.getName()]->isChecked()) {
            pipInstallCmd += " " + dep.getName();
        }
    }

    if (!settings.contains(SettingsKeys::INSTALLED_VERSION)) {
        urls.append("https://www.python.org/ftp/python/3.10.3/python-3.10.3-embed-amd64.zip");
        urls.append("https://bootstrap.pypa.io/get-pip.py");

        processes.append(m_applFolder + "\\python.exe " + m_tempFolder + "\\get-pip.py");
        processes.append(pipInstallCmd);
        processes.append(m_applFolder +
                         "\\python.exe -m pip install https://github.com/Dummerle/legendary/archive/refs/heads/rare.zip");
    }
    if(!QFile(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "\\Rare.lnk").exists()){
        processes.append(m_cmdFile->fileName() + " -m rare --desktop-shortcut");
    }

    QFile python_exe(m_applFolder + "\\python.exe");
    if (python_exe.exists()) {
        processProcess(processes.takeFirst());
    } else {
        downloader.download_files(urls);
    }
}

void RareUpdater::processProcess(const QString &executable) {
    ui->status_label->setText(tr("Running: ") + executable);
    install_process = new QProcess(this);
    install_process->setProcessChannelMode(QProcess::MergedChannels);
    install_process->setProgram(executable.split(" ")[0]);
    QStringList proc_args;
    for (const auto &arg: executable.split(" ").mid(1)) {

        proc_args.append(arg);
    }
    install_process->setArguments(proc_args);
    connect(install_process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(processFinished(int, QProcess::ExitStatus)));
    connect(install_process, SIGNAL(readyReadStandardOutput()),
            this, SLOT(installLogs()));
    install_process->start();
}

void RareUpdater::installLogs() {
    qDebug() << QString(install_process->readAllStandardOutput());
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
    for (auto &dep: cfg.opt_dependencies) {
        settings.setValue(SettingsKeys::get_name_for_dependency(dep.getName()), checkboxes[dep.getName()]->isChecked());
    }

    ui->page_stack->setCurrentIndex(pages.SUCCESS);
}

void RareUpdater::launch() {
    qDebug() << "launch";
    if (m_cmdFile->exists()) {
        m_proc->setProgram(m_cmdFile->fileName());
        m_proc->setArguments(QString("-m rare").split(" "));
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
    if (reply == 0) {
        qDebug() << "Cancel uninstall";
        return;
    }
    QDir app_dir(m_applFolder);
    app_dir.removeRecursively();
    settings.remove(SettingsKeys::INSTALLED_VERSION);
    for (auto &dep: cfg.opt_dependencies) {
        settings.remove(SettingsKeys::get_name_for_dependency(dep.getName()));
    }

    if (reply == 1) {
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
