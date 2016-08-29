#include "SceneViewModule.h"
#include "LibraryModule.h"

#include "TArcCore/TArcCore.h"

#include "Engine/Public/Engine.h"
#include "FileSystem/KeyedArchive.h"
#include "Render/RHI/rhi_Type.h"
#include "Base/BaseTypes.h"

int GameMain(DAVA::Vector<DAVA::String> cmdline)
{
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
        "PackManager"
    };

    DAVA::Engine e;
    DAVA::TArc::Core core(e);
    core.CreateModule<LibraryModule>();
    core.CreateModule<SceneViewModule>();

    e.SetOptions(appOptions);
    e.Init(DAVA::eEngineRunMode::GUI_EMBEDDED, modules);
    return e.Run();
}
