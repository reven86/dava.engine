#include "Engine/Public/Engine.h"
#include "Engine/Public/EngineContext.h"

#include "Logger/Logger.h"

#include "Particles/ParticleEmitter.h"
#include "FileSystem/FileSystem.h"

#include "EditorCore.h"

#include "TextureCompression/PVRConverter.h"
#include "QtTools/Utils/MessageHandler.h"
#include "QtTools/Utils/AssertGuard.h"
#include "QtTools/Utils/Themes/Themes.h"

#include <QtGlobal>
#include <QApplication>

void InitPVRTexTool()
{
#if defined(__DAVAENGINE_MACOS__)
    const DAVA::String pvrTexToolPath = "~res:/PVRTexToolCLI";
#elif defined(__DAVAENGINE_WIN32__)
    const DAVA::String pvrTexToolPath = "~res:/PVRTexToolCLI.exe";
#endif
    DAVA::PVRConverter::Instance()->SetPVRTexTool(pvrTexToolPath);
}

int GameMain(DAVA::Vector<DAVA::String> cmdline)
{
    DAVA::ParticleEmitter::FORCE_DEEP_CLONE = true;

    ToolsAssetGuard::Instance()->Init();

    DAVA::Engine engine;

    DAVA::KeyedArchive* appOptions = new DAVA::KeyedArchive();
    appOptions->SetString("title", "TemplateTArc");
    appOptions->SetInt32("fullscreen", 0);
    appOptions->SetInt32("bpp", 32);
    appOptions->SetInt32("rhi_threaded_frame_count", 1);
    appOptions->SetInt32("width", 1024);
    appOptions->SetInt32("height", 768);
    appOptions->SetInt32("renderer", rhi::RHI_GLES2);

    DAVA::Vector<DAVA::String> modules = {
        "JobManager",
        "NetCore",
        "LocalizationSystem",
        "SoundSystem",
        "DownloadManager",
    };
    engine.SetOptions(appOptions);
    engine.Init(DAVA::eEngineRunMode::GUI_EMBEDDED, modules);

    DAVA::EngineContext* context = engine.GetContext();

    context->logger->SetLogFilename("QuickEd.txt");

    const char* settingsPath = "QuickEdSettings.archive";
    DAVA::FilePath localPrefrencesPath(context->fileSystem->GetCurrentDocumentsDirectory() + settingsPath);
    PreferencesStorage::Instance()->SetupStoragePath(localPrefrencesPath);

    EditorCore* editorCore = nullptr;

    //we need to do it after QApplication will be created
    engine.gameLoopStarted.Connect([editorCore, &engine]() mutable {
        qInstallMessageHandler(DAVAMessageHandler);

        Q_INIT_RESOURCE(QtToolsResources);
        InitPVRTexTool();

        Themes::InitFromQApplication();

        editorCore = new EditorCore(engine);
        editorCore->Start();
    });

    engine.gameLoopStopped.Connect([editorCore]() {
        delete editorCore;
    });

    return engine.Run();
}
