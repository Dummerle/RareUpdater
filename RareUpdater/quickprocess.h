#ifndef QUICKPROCESS_H
#define QUICKPROCESS_H

#include <QProcess>
#include <QQuickItem>
#include <QVariant>

class QuickProcess : public QProcess
{
    Q_OBJECT
public:
    explicit QuickProcess(QObject *parent = 0);
    ~QuickProcess();

    Q_INVOKABLE void start(const QString &program, const QVariantList &arguments);
    Q_INVOKABLE QString readAll();
};

#endif // QUICKPROCESS_H
