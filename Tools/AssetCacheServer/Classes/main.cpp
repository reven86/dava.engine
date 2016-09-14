#include "Core/Core.h"
#include "Logger/Logger.h"
#include "QtTools/RunGuard/RunGuard.h"

#include "ServerCore.h"
#include "UI/AssetCacheServerWindow.h"

#include <QApplication>
#include <QCryptographicHash>

#include "Network/NetCore.h"

void FrameworkWillTerminate()
{
}

void FrameworkDidLaunched()
{
}

int main(int argc, char* argv[])
{
    DAVA::Core::Run(argc, argv);
    QApplication a(argc, argv);

    const QString appUid = "{DAVA.AssetCacheServer.Version.1.0.0}";
    const QString appUidPath = QCryptographicHash::hash((appUid).toUtf8(), QCryptographicHash::Sha1).toHex();
    RunGuard runGuard(appUidPath);
    if (runGuard.tryToRun())
    {
        DAVA::Logger::Instance()->SetLogFilename("AssetCacheServer.txt");
        DAVA::Logger::Instance()->SetLogLevel(DAVA::Logger::LEVEL_FRAMEWORK);

        ServerCore server;
        server.SetApplicationPath(QApplication::applicationFilePath().toStdString());
        AssetCacheServerWindow mainWindow(server);

        if (server.Settings().IsFirstLaunch())
        {
            mainWindow.OnFirstLaunch();
        }

        if (server.Settings().IsAutoStart())
        {
            server.Start();
        }

        return a.exec();
    }

    return -1;
}
