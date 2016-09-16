#include "Logger/Logger.h"
#include "QtHelpers/RunGuard.h"

#include "ServerCore.h"
#include "UI/AssetCacheServerWindow.h"

#include <QApplication>
#include <QCryptographicHash>

#include "Engine/Engine.h"
#include "Engine/EngineContext.h"

#include "Network/NetCore.h"

using namespace DAVA;

int Process(Engine& e)
{
    const QString appUid = "{DAVA.AssetCacheServer.Version.1.0.0}";
    const QString appUidPath = QCryptographicHash::hash((appUid).toUtf8(), QCryptographicHash::Sha1).toHex();
    std::unique_ptr<QtHelpers::RunGuard> runGuard = std::make_unique<QtHelpers::RunGuard>(appUidPath);
    if (!runGuard->tryToRun())
    {
        return -1;
    }

    Vector<char*> argv = e.GetCommandLineAsArgv();
    int argc = static_cast<int>(argv.size());

    QApplication a(argc, argv.data());

    EngineContext* context = e.GetContext();
    context->logger->SetLogFilename("AssetCacheServer.txt");
    context->logger->SetLogLevel(DAVA::Logger::LEVEL_FRAMEWORK);

    std::unique_ptr<ServerCore> server = std::make_unique<ServerCore>();
    server->SetApplicationPath(QApplication::applicationFilePath().toStdString());
    std::unique_ptr<AssetCacheServerWindow> mainWindow = std::make_unique<AssetCacheServerWindow>(*server);

    if (server->Settings().IsFirstLaunch())
    {
        mainWindow->OnFirstLaunch();
    }

    if (server->Settings().IsAutoStart())
    {
        server->Start();
    }

    QObject::connect(&a, &QApplication::aboutToQuit, [&]()
                     {
                         mainWindow.reset();
                         server.reset();
                         runGuard.reset();
                     });

    return a.exec();
}

int GameMain(DAVA::Vector<DAVA::String> cmdLine)
{
    Vector<String> modules =
    {
      "JobManager",
      "NetCore"
    };
    Engine e;
    e.Init(eEngineRunMode::CONSOLE_MODE, modules);

    e.update.Connect([&e](float32)
                     {
                         int result = Process(e);
                         e.Quit(result);
                     });

    return e.Run();
}
