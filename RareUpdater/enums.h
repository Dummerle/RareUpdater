#ifndef ENUMS_H
#define ENUMS_H

#include <QString>

enum DialogPages_t {
    LOADING = 0,
    SETTINGS,
    INSTALLED,
    UPDATE,
    SUCCESS,
};
typedef DialogPages_t DialogPages;


namespace SettingsKeys{

    static QString INSTALLED_VERSION("installer/installed_version");
    static QString get_name_for_dependency(const QString& dep){
        return QString("installer/" + dep + "_installed");
    }
};

#endif // ENUMS_H
