#include "downloader.h"

Downloader::Downloader(QObject *parent)
        : QObject{parent} {
    m_manager = new QNetworkAccessManager(this);
    m_manager->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
    connect(this->m_manager, SIGNAL(finished(QNetworkReply * )),
            this, SLOT(downloadFinished(QNetworkReply * )));
    m_applFolder = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    m_tempFolder = QStandardPaths::writableLocation(QStandardPaths::TempLocation);

}

void Downloader::download_files(const QStringList& urls) {
    qDebug() << urls;
    for (const auto &u: urls) {
        m_reqList.append(QNetworkRequest(u));
    }
    if(m_reqList.empty()){
        return;
    }
    process_request(m_reqList.takeFirst());
}

void Downloader::process_request(const QNetworkRequest &request) {
    qDebug() << m_tempFolder;
    m_downloadFile = new QFile(m_tempFolder + '\\' + request.url().fileName());
    if (m_downloadFile->exists())
        qDebug() << "Remove " << m_downloadFile->fileName();
        m_downloadFile->remove();
    if (!m_downloadFile->open(QIODevice::WriteOnly)){
        qDebug() << m_downloadFile->fileName();
        qDebug() << m_downloadFile->errorString();
        return;
    }
    // ui->status_label->setText(tr("Downloading ") + request.url().toString());
    emit current_download_changed(request.url().toString());
    m_reply = m_manager->get(request);
    connect(this->m_reply, SIGNAL(error(QNetworkReply::NetworkError)), this,
            SLOT(downloadError(QNetworkReply::NetworkError)));
    connect(this->m_reply, SIGNAL(readyRead()), this, SLOT(downloadReadyRead()));
    connect(this->m_reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadProgress(qint64, qint64)));

    // connect network reply signals

}

void Downloader::downloadError(QNetworkReply::NetworkError err) {
    qDebug() << err;
    m_reply->deleteLater();
    m_downloadFile->close();
    if (m_downloadFile->exists())
        m_downloadFile->remove();
}

void Downloader::downloadReadyRead() {
    QByteArray contents = m_reply->readAll();
    QDataStream stream(m_downloadFile);
    stream.writeRawData(contents, contents.size());
}

void Downloader::downloadProgress(qint64 read, qint64 total) {
    //ui->download_progress->setMaximum(total);
    //ui->download_progress->setValue(read);
    qDebug() << "Downloader" << (int)((read * 100) / total);
    emit progress_update((int)((read * 100) / total));
}

bool Downloader::isHttpRedirect(QNetworkReply *reply) {
    qDebug() << reply->url();
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    return statusCode == 301 || statusCode == 302 || statusCode == 303
           || statusCode == 305 || statusCode == 307 || statusCode == 308;
}

void Downloader::downloadFinished(QNetworkReply *reply) {
    qDebug() << "Finished " << reply->url();
    if (reply == m_reply) {
        if (isHttpRedirect(reply))
            m_reqList.append(QNetworkRequest(reply->url()));
        m_reply->close();
        m_reply->deleteLater();
    }
    m_downloadFile->close();

    if (!m_reqList.isEmpty()) {
        process_request(m_reqList.takeFirst());
        return;
    }

    QDir appFolder(m_applFolder);
    if (appFolder.exists())
        appFolder.removeRecursively();
    qDebug() << m_applFolder;
    JlCompress::extractDir(
            m_tempFolder + "\\python-3.10.3-embed-amd64.zip",
            m_applFolder
    );
    QFile pth(m_applFolder + "\\python310._pth");
    if (pth.open(QIODevice::WriteOnly | QIODevice::Append)) {
        pth.write("import site");
        pth.close();
    }

    // processProcess(processes.takeFirst());
}

