#include <QString>
#include <QStringList>
#include <QDir>

namespace Utils {
    QString formatSize(qint64 size) {
        QStringList units = {"Bytes", "KB", "MB", "GB", "TB", "PB"};
        int i;
        double outputSize = size;
        for (i = 0; i < units.size() - 1; i++) {
            if (outputSize < 1024) break;
            outputSize = outputSize / 1024;
        }
        return QString("%0 %1").arg(outputSize, 0, 'f', 2).arg(units[i]);
    }

    qint64 dirSize(const QString& dirPath) {
        qint64 size = 0;
        QDir dir(dirPath);
        //calculate total size of current directories' files
        QDir::Filters fileFilters = QDir::Files|QDir::System|QDir::Hidden;
        for(const QString& filePath : dir.entryList(fileFilters)) {
            QFileInfo fi(dir, filePath);
            size+= fi.size();
        }
        //add size of child directories recursively
        QDir::Filters dirFilters = QDir::Dirs|QDir::NoDotAndDotDot|QDir::System|QDir::Hidden;
        for(const QString& childDirPath : dir.entryList(dirFilters))
            size+= dirSize(dirPath + QDir::separator() + childDirPath);
        return size;
    }

}