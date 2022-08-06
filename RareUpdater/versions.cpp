#include "versions.hpp"

#include <QJsonParseError>
#include <QJsonObject>

Versions::Versions(QString url, Component comp, QObject *parent)
    : QObject{parent}
{
    m_manager = new QNetworkAccessManager(this);
    m_manager->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);

    m_url.setUrl(url);
    m_component = comp;

    connect(this->m_manager, &QNetworkAccessManager::finished,
            this, &Versions::parse);
}

Versions::~Versions()
{
    delete m_manager;
}

void Versions::fetch()
{
    m_versions.clear();
    m_manager->get(QNetworkRequest(m_url));
}

bool Versions::error(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(reply->url().toString(), reply->errorString());
        reply->close();
        reply->deleteLater();
        return true;
    }
    return false;
}

bool Versions::redirect(QNetworkReply *reply) {
    qDebug() << "redirect: "<< reply->url();
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode == 301 || statusCode == 302 || statusCode == 303 ||
        statusCode == 305 || statusCode == 307 || statusCode == 308)
    {
        m_url = reply->url();
        reply->close();
        reply->deleteLater();
        fetch();
        return true;
    }
    return false;
}

void Versions::parse(QNetworkReply *reply) {
    if (redirect(reply))
        return;
    if (error(reply))
        return;

    qDebug() << reply->url();

    QByteArray data = reply->readAll().constData();
    reply->close();
    reply->deleteLater();

    if (m_component == Component::Rare)
        m_versions_ready = parseRareVersions(reply, &data, &m_versions);
    if (m_component == Component::Python)
        m_versions_ready = parsePythonVersions(reply, &data, &m_versions);

    if (m_versions_ready)
        emit finished();
}

bool Versions::parseRareVersions(QNetworkReply *reply, QByteArray *data, QStringList *list)
{
    QJsonParseError error;
    QJsonDocument json(QJsonDocument::fromJson(*data, &error));
    m_versions = json.object()["releases"].toObject().keys();
    std::reverse(m_versions.begin(), m_versions.end());

    return !m_versions.isEmpty();
}

bool Versions::parsePythonVersions(QNetworkReply *reply, QByteArray *data, QStringList *list)
{
    QString doc(*data);
    // The regex intentionally doesn't match 0 minor and non-bugfix versions
    QString pattern("<a href=\"(3\\.[1-9][0-9]*\\.[1-9]+)\\/\">");

    static QRegularExpression re(pattern);
    QRegularExpressionMatchIterator match = re.globalMatch(doc);
    while (match.hasNext()) {
        m_versions.append(match.next().captured(1));
    }
    std::sort(m_versions.begin(), m_versions.end(), &Versions::comparePythonVersions);

    return !m_versions.isEmpty();
}

bool Versions::comparePythonVersions(const QString& a, const QString& b)
{
    QStringList ta = a.split(".");
    QStringList tb = b.split(".");

    int ta_v = (ta[0].toInt() << 16) + (ta[1].toInt() << 8) + ta[2].toInt();
    int tb_v = (tb[0].toInt() << 16) + (tb[1].toInt() << 8) + tb[2].toInt();

    if (ta_v < tb_v)
        return false;
    return true;
}


