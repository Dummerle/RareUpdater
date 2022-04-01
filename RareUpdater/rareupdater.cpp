#include "rareupdater.h"
#include "ui_rareupdater.h"

RareLauncher::RareLauncher(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RareLauncher)
{
    ui->setupUi(this);
    m_proc = new QProcess(this);
    m_manager = new QNetworkAccessManager(this);
    m_manager->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);

    m_applFolder = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    m_tempFolder = QStandardPaths::writableLocation(QStandardPaths::TempLocation);

    m_cmdFile = new QFile(m_applFolder + "\\Scripts\\rare.exe");
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(m_cmdFile->exists());

    ui->extra_space_lbl->setText(ui->extra_space_lbl->text().replace("{}", "0MB"));

    connect(ui->install, SIGNAL(clicked()), this, SLOT(install()));
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(launch()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(cancel()));
}

RareLauncher::~RareLauncher()
{
    delete ui;
}

void RareLauncher::install()
{
    ui->install->setDisabled(true);
    QStringList urls;
    urls.append("https://bootstrap.pypa.io/get-pip.py");
    urls.append("https://www.python.org/ftp/python/3.10.3/python-3.10.3-embed-amd64.zip");

    for (const auto& u: urls) {
        m_reqList.append(QNetworkRequest(u));
    }

    processRequest(m_reqList.takeFirst());
}

void RareLauncher::processRequest(QNetworkRequest request)
{
    m_downloadFile = new QFile(m_tempFolder + '\\' + request.url().fileName());
    qDebug() << m_downloadFile->fileName();
    if (m_downloadFile->exists())
        m_downloadFile->remove();
    m_downloadFile->open(QIODevice::WriteOnly | QIODevice::NewOnly);
    m_reply = m_manager->get(request);
    // connect network reply signals
    connect(this->m_reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(downloadError(QNetworkReply::NetworkError)));
    connect(this->m_reply, SIGNAL(readyRead()), this, SLOT(downloadReadyRead()));
    connect(this->m_reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));
    connect(this->m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(downloadFinished(QNetworkReply*)));
}

void RareLauncher::downloadError(QNetworkReply::NetworkError err)
{
    qDebug() << err;
    m_reply->deleteLater();
    m_downloadFile->close();
    if (m_downloadFile->exists())
        m_downloadFile->remove();
}

void RareLauncher::downloadReadyRead()
{
    QByteArray contents = m_reply->readAll();
    QDataStream stream(m_downloadFile);
    stream.writeRawData(contents, contents.size());
}

void RareLauncher::downloadProgress(qint64 read, qint64 total)
{
    qDebug() << QString("read:%1 total:%2").arg(read).arg(total);
    ui->download_progress->setMaximum(total);
    ui->download_progress->setValue(read);
}

bool RareLauncher::isHttpRedirect(QNetworkReply *reply)
{
    qDebug() << reply->url();
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    return statusCode == 301 || statusCode == 302 || statusCode == 303
           || statusCode == 305 || statusCode == 307 || statusCode == 308;
}

void RareLauncher::downloadFinished(QNetworkReply* reply)
{
    if (reply == m_reply) {
        if (isHttpRedirect(reply))
            m_reqList.append(QNetworkRequest(reply->url()));
        m_reply->close();
        m_reply->deleteLater();
    }
    m_downloadFile->close();

    if (!m_reqList.isEmpty()) {
        processRequest(m_reqList.takeFirst());
    } else {
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

        QProcess get_pip;
        get_pip.setProgram(m_applFolder + "\\python.exe");
        QStringList get_pip_args;
        get_pip_args.append(QString(m_tempFolder + "\\get-pip.py"));
        get_pip.setArguments(get_pip_args);
        get_pip.start();
        get_pip.waitForFinished();
        get_pip.deleteLater();

        QProcess pip;
        pip.setProgram(m_applFolder + "\\Scripts\\pip.exe");
        QStringList pip_args;
        pip_args.append(QString("install"));
        pip_args.append(QString("Rare"));
        pip.setArguments(pip_args);
        pip.start();
        pip.waitForFinished();
        pip.deleteLater();

        launch();
    }
}

void RareLauncher::launch()
{
    if (m_cmdFile->exists()) m_proc->setProgram(m_cmdFile->fileName());
    bool ret = m_proc->startDetached();
    if (ret) qApp->exit();
}

void RareLauncher::cancel()
{
    qApp->exit();
}
