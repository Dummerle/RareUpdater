#include "rareupdater.h"
#include "ui_rareupdater.h"
#include <QMessageBox>

RareUpdater::RareUpdater(QWidget *parent)
    : QDialog(parent), ui(new Ui::RareUpdater) {
    ui->setupUi(this);
    m_proc = new QProcess(this);
    m_manager = new QNetworkAccessManager(this);
    m_manager->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);

    m_applFolder = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    m_tempFolder = QStandardPaths::writableLocation(QStandardPaths::TempLocation);

    m_cmdFile = new QFile(m_applFolder + "\\pythonw.exe");
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(m_cmdFile->exists());

    ui->extra_space_lbl->setText(ui->extra_space_lbl->text().replace("{}", "0MB"));

    connect(ui->install, SIGNAL(clicked()), this, SLOT(install()));
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(launch()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(cancel()));

    connect(this->m_manager, SIGNAL(finished(QNetworkReply * )), this, SLOT(downloadFinished(QNetworkReply * )));
    connect(ui->launch_button, SIGNAL(clicked()), this, SLOT(launch()));
}

RareUpdater::~RareUpdater() {
    delete ui;
}

void RareUpdater::install() {
    ui->install->setDisabled(true);
    QStringList urls;
    urls.append("https://bootstrap.pypa.io/get-pip.py");
    urls.append("https://www.python.org/ftp/python/3.10.3/python-3.10.3-embed-amd64.zip");

    for (const auto &u: urls) {
        m_reqList.append(QNetworkRequest(u));
    }


    processRequest(m_reqList.takeFirst());
}

void RareUpdater::processRequest(QNetworkRequest request) {
    m_downloadFile = new QFile(m_tempFolder + '\\' + request.url().fileName());
    if (m_downloadFile->exists())
        m_downloadFile->remove();
    m_downloadFile->open(QIODevice::WriteOnly | QIODevice::NewOnly);
    ui->status_label->setText(tr("Downloading ") + request.url().toString());
    m_reply = m_manager->get(request);
    connect(this->m_reply, SIGNAL(error(QNetworkReply::NetworkError)), this,
            SLOT(downloadError(QNetworkReply::NetworkError)));
    connect(this->m_reply, SIGNAL(readyRead()), this, SLOT(downloadReadyRead()));
    connect(this->m_reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadProgress(qint64, qint64)));

    // connect network reply signals

}

void RareUpdater::downloadError(QNetworkReply::NetworkError err) {
    qDebug() << err;
    m_reply->deleteLater();
    m_downloadFile->close();
    if (m_downloadFile->exists())
        m_downloadFile->remove();
}

void RareUpdater::downloadReadyRead() {
    QByteArray contents = m_reply->readAll();
    QDataStream stream(m_downloadFile);
    stream.writeRawData(contents, contents.size());
}

void RareUpdater::downloadProgress(qint64 read, qint64 total) {
    ui->download_progress->setMaximum(total);
    ui->download_progress->setValue(read);
}

bool RareUpdater::isHttpRedirect(QNetworkReply *reply) {
    qDebug() << reply->url();
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    return statusCode == 301 || statusCode == 302 || statusCode == 303
            || statusCode == 305 || statusCode == 307 || statusCode == 308;
}

void RareUpdater::downloadFinished(QNetworkReply *reply) {
    if (reply == m_reply) {
        if (isHttpRedirect(reply))
            m_reqList.append(QNetworkRequest(reply->url()));
        m_reply->close();
        m_reply->deleteLater();
    }
    m_downloadFile->close();

    if (!m_reqList.isEmpty()) {
        processRequest(m_reqList.takeFirst());
        return;
    }
    QDir appFolder(m_applFolder);
    if (appFolder.exists())
        appFolder.removeRecursively();

    JlCompress::extractDir(
                m_tempFolder + "\\python-3.10.3-embed-amd64.zip",
                m_applFolder
                );

    QFile pth(m_applFolder + "\\python310._pth");
    if (pth.open(QIODevice::WriteOnly | QIODevice::Append)) {
        pth.write("import site");
        pth.close();
    }

    installPip();

}

void RareUpdater::installPip() {
    ui->status_label->setText(tr("Installing pip"));
    get_pip_process = new QProcess(this);
    get_pip_process->setProgram(m_applFolder + "\\python.exe");
    QStringList get_pip_args;
    get_pip_args.append(QString(m_tempFolder + "\\get-pip.py"));
    get_pip_process->setArguments(get_pip_args);
    connect(get_pip_process, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(installRare(int,QProcess::ExitStatus)));
    get_pip_process->start();

}

void RareUpdater::installRare(int exit_code, QProcess::ExitStatus e) {
    if(exit_code != 0){
        return;
    }
    ui->status_label->setText(tr("Installing Rare and dependencies"));
    sender()->deleteLater();
    pip_process = new QProcess(this);
    pip_process->setProgram(m_applFolder + "\\Scripts\\pip.exe");
    QStringList pip_args;
    pip_args.append(QString("install"));
    pip_args.append(QString("Rare"));
    pip_process->setArguments(pip_args);
    pip_process->start();
    connect(pip_process, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(launch(int,QProcess::ExitStatus)));

}


void RareUpdater::launch(int exit_code, QProcess::ExitStatus e) {
    pip_process->deleteLater();
    ui->status_label->setText(tr("Launching"));
    ui->page_stack->setCurrentIndex(1);
}

void RareUpdater::launch() {
    if (m_cmdFile->exists()){
        m_proc->setProgram(m_cmdFile->fileName());
        m_proc->setArguments(QString("-m rare").split(" "));
    }
    bool ret = m_proc->startDetached();
    if (ret) qApp->exit();
}

void RareUpdater::cancel() {
    qApp->exit();
}
