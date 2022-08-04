#include "downloader.h"

Downloader::Downloader(
        const QString& data_folder, const QString& temp_folder, QObject *parent) :
    QObject{parent}
{
    m_manager = new QNetworkAccessManager(this);
    m_manager->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
    m_data_folder = data_folder;
    m_temp_folder = temp_folder;

    connect(this->m_manager, &QNetworkAccessManager::finished,
            this, &Downloader::downloadFinished);
}

void Downloader::downloadFiles(const QStringList& urls) {
    if (urls.empty()) {
        qDebug() << "urls: empty";
        return;
    }
    qDebug() << urls;
    for (const auto &u: urls) {
        m_requests.append(QNetworkRequest(u));
    }
    processRequest(m_requests.takeFirst());
}

void Downloader::processRequest(const QNetworkRequest &request) {
    qDebug() << "temp dir: " << m_temp_folder;
    m_download_file = new QFile(m_temp_folder + '/' + request.url().fileName());
    qDebug() << "temp file: " << m_download_file->fileName();

    if (m_download_file->exists()) {
        qDebug() << "old file: " << m_download_file->fileName();
        m_download_file->remove();
        qDebug() << "removed: " << m_download_file->fileName();
    }

    if (!m_download_file->open(QIODevice::WriteOnly)) {
        qDebug() << m_download_file->fileName();
        qDebug() << m_download_file->errorString();
        return;
    }

    emit current(request.url().toString());

    m_reply = m_manager->get(request);

    connect(this->m_reply, &QNetworkReply::errorOccurred,
            this, &Downloader::downloadError);
    connect(this->m_reply, &QNetworkReply::readyRead,
            this, &Downloader::downloadReadyRead);
    connect(this->m_reply, &QNetworkReply::downloadProgress,
            this, &Downloader::downloadProgress);

    // connect network reply signals

}

void Downloader::downloadError(QNetworkReply::NetworkError err) {
    qDebug() << err;
    m_reply->deleteLater();
    m_download_file->close();
    if (m_download_file->exists())
        m_download_file->remove();
}

void Downloader::downloadReadyRead() {
    QByteArray contents = m_reply->readAll();
    QDataStream stream(m_download_file);
    stream.writeRawData(contents, contents.size());
}

void Downloader::downloadProgress(qint64 read, qint64 total) {
    qDebug() << "progress: " << (int)((read * 100) / total);
    emit progress((int)((read * 100) / total));
}

bool Downloader::isHttpRedirect(QNetworkReply *reply) {
    qDebug() << "redirect: "<< reply->url();
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    return statusCode == 301 || statusCode == 302 || statusCode == 303
           || statusCode == 305 || statusCode == 307 || statusCode == 308;
}

void Downloader::downloadFinished(QNetworkReply *reply) {
    qDebug() << "finished: " << reply->url();
    if (reply == m_reply) {
        if (isHttpRedirect(reply))
            m_requests.append(QNetworkRequest(reply->url()));
        m_reply->close();
        m_reply->deleteLater();
    }
    m_download_file->close();

    if (!m_requests.isEmpty()) {
        processRequest(m_requests.takeFirst());
        return;
    }

    emit finished();
}

