#include "rareupdater.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "RareLauncher_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    a.setStyle(QStyleFactory::create("Fusion"));
        QFile f(":/stylesheets/Quark/stylesheet.qss");
        if (f.open(QFile::ReadOnly)) {
            QString stylesheet(f.readAll());
            a.setStyleSheet(stylesheet);
        }


    a.setOrganizationName("Rare");
    a.setApplicationName("Rare");
    bool is_modify = QString(*argv).contains("modify");
    QString app_arg;
    if(is_modify)
        app_arg = "modify";

    RareUpdater w(app_arg);
    w.show();
    return a.exec();
}
