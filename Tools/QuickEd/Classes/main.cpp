#include "Engine/Engine.h"
#include "EditorCore.h"

DAVA::KeyedArchive* CreateOptions()
{
    DAVA::KeyedArchive* appOptions = new DAVA::KeyedArchive();
    appOptions->SetString("title", "QuickEd");
    appOptions->SetInt32("fullscreen", 0);
    appOptions->SetInt32("bpp", 32);
    appOptions->SetInt32("rhi_threaded_frame_count", 1);
    appOptions->SetInt32("width", 1024);
    appOptions->SetInt32("height", 768);
    appOptions->SetInt32("renderer", rhi::RHI_GLES2);
    return appOptions;
}

int GameMain(DAVA::Vector<DAVA::String> cmdline)
{
    DAVA::Engine engine;
    {
        DAVA::Vector<DAVA::String> modules =
        {
          "JobManager",
          "NetCore",
          "LocalizationSystem",
          "SoundSystem",
          "DownloadManager",
        };
        DAVA::ScopedPtr<DAVA::KeyedArchive> options(CreateOptions());
        engine.SetOptions(options);
        engine.Init(DAVA::eEngineRunMode::GUI_EMBEDDED, modules);
    }

    std::unique_ptr<EditorCore> editorCore;
    engine.gameLoopStarted.Connect([&]()
                                   {
                                       DVASSERT(editorCore == nullptr);
                                       editorCore = std::make_unique<EditorCore>();
                                       editorCore->Init(engine);
                                   });

    engine.windowCreated.Connect([&](DAVA::Window&)
                                 {
                                     DVASSERT(editorCore != nullptr);
                                     editorCore->OnRenderingInitialized();
                                 });

    engine.gameLoopStopped.Connect([&]()
                                   {
                                       DVASSERT(editorCore != nullptr);
                                       editorCore.reset();
                                   });

    return engine.Run();
}
