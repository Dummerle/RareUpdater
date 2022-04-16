#include <QString>
#include "string"


class DialogPageIndexes{
public:
    int LOADING = 0;
    int SETTINGS = 1;
    int INSTALLED = 2;
    int UPDATE = 3;
    int SUCCESS = 4;
};

namespace SettingsKeys{

    static QString INSTALLED_VERSION("installer/installed_version");
    static QString get_name_for_dependency(const QString& dep){
        return QString("installer/" + dep + "_installed");
    }
};
