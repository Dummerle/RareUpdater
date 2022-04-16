
#include <QList>
#include <utility>

class OptDependency{
    QString name;
    QString info_text;
public:
    OptDependency(QString p_name, QString p_info_text){
        name = std::move(p_name);
        info_text = std::move(p_info_text);
    };
    QString getName(){
        return name;
    }
    QString getInfo(){
        return info_text;
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