#pragma once

#include <QObject>
#include <QLocalSocket>
#include "QtHelpers/LauncherListener.h"

class AppsCommandsSender : public QObject
{
    Q_OBJECT

public:
    AppsCommandsSender(QObject* parent = nullptr);
    ~AppsCommandsSender();

    bool HostIsAvailable(const QString& appPath);
    bool Ping(const QString& path);
    bool RequestQuit(const QString& appPath);

private:
    long SendMessage(long message, const QString& appPath);
    QLocalSocket* socket = nullptr;
};
