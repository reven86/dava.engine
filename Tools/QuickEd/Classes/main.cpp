#include "Engine/Engine.h"
#include "EditorCore.h"
#include "Render/RHI/rhi_Type.h"

DAVA::KeyedArchive* CreateOptions()
{
    DAVA::KeyedArchive* appOptions = new DAVA::KeyedArchive();
    appOptions->SetInt32("bpp", 32);
    appOptions->SetInt32("rhi_threaded_frame_count", 1);
    appOptions->SetInt32("renderer", rhi::RHI_GLES2);
    appOptions->SetBool("trackFont", true);
    return appOptions;
}

int DAVAMain(DAVA::Vector<DAVA::String> cmdline)
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
        engine.Init(DAVA::eEngineRunMode::GUI_EMBEDDED, modules, CreateOptions());
    }

    std::unique_ptr<EditorCore> editorCore;
    engine.gameLoopStarted.Connect([&]()
                                   {
                                       DVASSERT(editorCore == nullptr);
                                       editorCore = std::make_unique<EditorCore>(engine);
                                   });

    engine.windowCreated.Connect([&](DAVA::Window*)
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
