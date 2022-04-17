#ifndef CONFIG_H
#define CONFIG_H

#include <QList>
#include <utility>

class OptDependency{
    QString m_name;
    QString m_info_text;
public:
    OptDependency(QString name, QString info_text){
        m_name = name;
        m_info_text = info_text;
    };

    inline const QString &name() const {
        return m_name;
    }

    inline const QString &info_text() const {
        return m_info_text;
    }
};

class Config{
public:
    QString pkg_name = "rare";
    QList<OptDependency> opt_dependencies = {
            OptDependency(QString("pypresence"), QString("(use Discord Rich Presence)")),
            OptDependency(QString("pywebview"), QString("(Add a browser window for a simpler login process)"))
    };
};
#endif // CONFIG_H
