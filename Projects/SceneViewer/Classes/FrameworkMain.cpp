#include "DAVAEngine.h"
#include "GameCore.h"
#include "Platform/DeviceInfo.h"

using namespace DAVA;

void FrameworkDidLaunched()
{    
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)

//for iPad
#define WIDTH 1024
#define HEIGHT 768

    KeyedArchive* appOptions = new KeyedArchive();
    appOptions->SetInt32("orientation", Core::SCREEN_ORIENTATION_LANDSCAPE_LEFT);

    appOptions->SetInt32("renderer", rhi::RHI_GLES2);
    //    appOptions->SetInt32("renderer", rhi::RHI_METAL);

    appOptions->SetInt32("rhi_threaded_frame_count", 2);

    appOptions->SetBool("iPhone_autodetectScreenScaleFactor", true);
    appOptions->SetInt32("width", WIDTH);
    appOptions->SetInt32("height", HEIGHT);

    DAVA::VirtualCoordinatesSystem::Instance()->SetVirtualScreenSize(WIDTH, HEIGHT);
    DAVA::VirtualCoordinatesSystem::Instance()->RegisterAvailableResourceSize(WIDTH, HEIGHT, "Gfx");
#elif defined(__DAVAENGINE_WIN_UAP__)

    KeyedArchive* appOptions = new KeyedArchive();

    appOptions->SetInt32("width", DeviceInfo::GetScreenInfo().width);
    appOptions->SetInt32("height", DeviceInfo::GetScreenInfo().height);

    appOptions->SetInt32("fullscreen", 0);
    appOptions->SetInt32("bpp", 32);
    appOptions->SetString(String("title"), String("Scene Viewer"));

    DAVA::VirtualCoordinatesSystem::Instance()->SetVirtualScreenSize(DeviceInfo::GetScreenInfo().width, DeviceInfo::GetScreenInfo().height);
    DAVA::VirtualCoordinatesSystem::Instance()->RegisterAvailableResourceSize(DeviceInfo::GetScreenInfo().width, DeviceInfo::GetScreenInfo().height, "Gfx");
    ;
    appOptions->SetInt32("renderer", rhi::RHI_DX11);
    appOptions->SetInt32("rhi_threaded_frame_count", 2);

#else
    KeyedArchive* appOptions = new KeyedArchive();
#if defined(__DAVAENGINE_WIN32__)
    appOptions->SetInt32("renderer", rhi::RHI_DX11);
    //    appOptions->SetInt32("renderer", rhi::RHI_DX9);
    //appOptions->SetInt32("renderer", rhi::RHI_GLES2);
    appOptions->SetInt32("rhi_threaded_frame_count", 2);
#elif defined(__DAVAENGINE_MACOS__)
    appOptions->SetInt32("renderer", rhi::RHI_GLES2);
#endif

    appOptions->SetInt32("width", 1024);
    appOptions->SetInt32("height", 768);

    // 	appOptions->SetInt("fullscreen.width",	1280);
    // 	appOptions->SetInt("fullscreen.height", 800);

    appOptions->SetInt32("fullscreen", 0);
    appOptions->SetInt32("bpp", 32);
    appOptions->SetString(String("title"), String("Scene Viewer"));

    DAVA::VirtualCoordinatesSystem::Instance()->SetVirtualScreenSize(1024, 768);
    DAVA::VirtualCoordinatesSystem::Instance()->RegisterAvailableResourceSize(1024, 768, "Gfx");
#endif

    GameCore* core = new GameCore();
    DAVA::Core::SetApplicationCore(core);
    DAVA::Core::Instance()->SetOptions(appOptions);
}

void FrameworkWillTerminate()
{
}
