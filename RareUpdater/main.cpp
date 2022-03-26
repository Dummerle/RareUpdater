#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <QLocale>
#include <QTranslator>
#include <string>

#include "backend.h"
#include "external/QuickDownload/src/quickdownload.h"
#include "quickprocess.h"

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QGuiApplication app(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "RareUpdater_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            app.installTranslator(&translator);
            break;
        }
    }

    qmlRegisterType<QuickDownload>("QuickDownload", 1, 0, "Download");
    qmlRegisterType<QuickProcess>("QuickProcess", 1, 0, "Process");

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    Backend backend;

    QList<QObject*> rootObjects = engine.rootObjects();
    QObject* first = rootObjects.first();
    QObject *install_python = first->findChild<QObject*>("install_button");

    QObject::connect(install_python, SIGNAL(clicked()),
                     &backend, SLOT(download_python()));

    return app.exec();
}
