#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include <QtDebug>

struct Version {
    QString name;
    QUrl url;
};

class Versions : public QObject
{
    Q_OBJECT
public:
    enum Component {
        Rare,
        Python,
    };

    explicit Versions(QString, Component, QObject *parent = nullptr);
    ~Versions() override;

    void fetch();
    QStringList all() { return m_versions; };
    QString latest() { return m_versions.constFirst(); };

signals:
    void finished();
    void errorOccurred(QString, QString);

private:
    void parse(QNetworkReply *reply);
    bool error(QNetworkReply *reply);

    bool parseRareVersions(QNetworkReply *reply, QByteArray *data, QStringList *list);
    bool parsePythonVersions(QNetworkReply *reply, QByteArray *data, QStringList *list);
    static bool comparePythonVersions(const QString& a, const QString& b);

    QUrl m_url;
    Component m_component;
    QNetworkAccessManager *m_manager;
    QStringList m_versions;
    bool m_versions_ready = false;
};

