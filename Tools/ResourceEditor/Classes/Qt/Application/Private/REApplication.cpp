#include "Classes/Qt/Application/REApplication.h"

#include "FileSystem/KeyedArchive.h"
#include "Base/BaseTypes.h"

#include "TArcCore/TArcCore.h"

namespace REApplicationDetail
{
DAVA::KeyedArchive* CreateOptions()
{
    DAVA::KeyedArchive* appOptions = new DAVA::KeyedArchive();

    appOptions->SetInt32("bpp", 32);
    appOptions->SetInt32("renderer", rhi::RHI_GLES2);
    appOptions->SetInt32("max_index_buffer_count", 16384);
    appOptions->SetInt32("max_vertex_buffer_count", 16384);
    appOptions->SetInt32("max_const_buffer_count", 32767);
    appOptions->SetInt32("max_texture_count", 2048);

    appOptions->SetInt32("shader_const_buffer_size", 256 * 1024 * 1024);

    return appOptions;
}
}

REApplication::REApplication(DAVA::Vector<DAVA::String>&& cmdLine_)
    : cmdLine(std::move(cmdLine_))
{
    /*
     // TODO
     if (something)
     {
        isConsoleMode = true;
     }
     */
}

DAVA::TArc::BaseApplication::EngineInitInfo REApplication::GetInitInfo() const
{
    EngineInitInfo initInfo;
    initInfo.runMode = isConsoleMode ? DAVA::eEngineRunMode::CONSOLE_MODE : DAVA::eEngineRunMode::GUI_EMBEDDED;
    initInfo.modules = DAVA::Vector<DAVA::String>
    {
      "JobManager",
      "NetCore",
      "LocalizationSystem",
      "SoundSystem",
      "DownloadManager",
    };

    initInfo.options.Set(REApplicationDetail::CreateOptions());
    return initInfo;
}

void REApplication::CreateModules(DAVA::TArc::Core* tarcCore) const
{
    if (isConsoleMode)
        CreateConsoleModules(tarcCore);
    else
        CreateGUIModules(tarcCore);
}

void REApplication::Cleanup()
{
    cmdLine.clear();
}

void REApplication::CreateGUIModules(DAVA::TArc::Core* tarcCore) const
{
    // TODO
}

void REApplication::CreateConsoleModules(DAVA::TArc::Core* tarcCore) const
{
    // TODO
}