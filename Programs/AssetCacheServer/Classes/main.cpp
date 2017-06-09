#include "UI/AssetCacheServerWindow.h"
#include "ServerCore.h"

#include "Engine/Engine.h"
#include "Engine/EngineContext.h"
#include "Logger/Logger.h"

#include "QtHelpers/RunGuard.h"
#include "QtHelpers/LauncherListener.h"

#include <QApplication>
#include <QCryptographicHash>

using namespace DAVA;

int Process(Engine& e)
{
    const QString appUid = "{DAVA.AssetCacheServer.Version.1.0.0}";
    const QString appUidPath = QCryptographicHash::hash((appUid).toUtf8(), QCryptographicHash::Sha1).toHex();
    std::unique_ptr<QtHelpers::RunGuard> runGuard = std::make_unique<QtHelpers::RunGuard>(appUidPath);
    if (!runGuard->TryToRun())
    {
        return -1;
    }

    Vector<char*> argv = e.GetCommandLineAsArgv();
    int argc = static_cast<int>(argv.size());

    QApplication a(argc, argv.data());

    const EngineContext* context = e.GetContext();
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

    LauncherListener launcherListener;
    launcherListener.Init([](LauncherListener::eMessage message) {
        if (message == LauncherListener::eMessage::QUIT)
        {
            Logger::Info("Got quit message from launcher listener");
            qApp->quit();
            return LauncherListener::eReply::ACCEPT;
        }
        return LauncherListener::eReply::UNKNOWN_MESSAGE;
    });

    return a.exec();
}

int DAVAMain(DAVA::Vector<DAVA::String> cmdLine)
{
    Vector<String> modules =
    {
      "JobManager",
      "NetCore"
    };

    KeyedArchive* options = new KeyedArchive; // options will be placed into RefPtr inside of Engine
    options->SetBool("separate_net_thread", true);

    Engine e;
    e.Init(eEngineRunMode::CONSOLE_MODE, modules, options);

    e.update.Connect([&e](float32)
                     {
                         int result = Process(e);
                         e.QuitAsync(result);
                     });

    return e.Run();
}
