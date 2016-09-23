#include "SceneViewModule.h"
#include "LibraryModule.h"
#include "ConsoleCommandModule.h"

#include "TArcCore/TArcCore.h"
#include "Testing/TArcTestCore.h"

#include "Engine/Engine.h"
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
        "DownloadManager"
    };

    DAVA::Engine e;
    e.SetOptions(appOptions);

    if (cmdline.size() > 1 && cmdline[1] == "selftest")
    {
        e.Init(DAVA::eEngineRunMode::GUI_EMBEDDED, modules);
        DAVA::TArc::TestCore testCore(e);
        return e.Run();
    }

    bool isConsoleMode = cmdline.size() > 1;
    e.Init(isConsoleMode ? DAVA::eEngineRunMode::CONSOLE_MODE : DAVA::eEngineRunMode::GUI_EMBEDDED, modules);
    DAVA::TArc::Core core(e);

    if (isConsoleMode)
    {
        int argv = static_cast<int>(cmdline.size());
        int currentArg = 1;
        while (currentArg < argv)
        {
            if (cmdline[currentArg] == "staticocclusion")
            {
                ++currentArg;
                DVASSERT(currentArg < argv);
                core.CreateModule<ConsoleCommandModule>(cmdline[currentArg]);
            }
            ++currentArg;
        }
    }
    else
    {
        core.CreateModule<LibraryModule>();
        core.CreateModule<SceneViewModule>();
    }

    return e.Run();
}
