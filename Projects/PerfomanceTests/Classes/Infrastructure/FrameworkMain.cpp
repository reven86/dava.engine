#include "DAVAEngine.h"
#include "GameCore.h"

using namespace DAVA;

void FrameworkDidLaunched()
{
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    
#define IOS_WIDTH 1024
#define IOS_HEIGHT 768

    KeyedArchive* appOptions = new KeyedArchive();
    appOptions->SetInt32("orientation", Core::SCREEN_ORIENTATION_LANDSCAPE_AUTOROTATE);

    appOptions->SetInt32("width", IOS_WIDTH);
    appOptions->SetInt32("height", IOS_HEIGHT);

    DAVA::VirtualCoordinatesSystem::Instance()->SetVirtualScreenSize(IOS_WIDTH, IOS_HEIGHT);
    DAVA::VirtualCoordinatesSystem::Instance()->SetProportionsIsFixed(false);
    DAVA::VirtualCoordinatesSystem::Instance()->RegisterAvailableResourceSize(IOS_WIDTH, IOS_HEIGHT, "Gfx");
    DAVA::VirtualCoordinatesSystem::Instance()->RegisterAvailableResourceSize(IOS_WIDTH * 2, IOS_HEIGHT * 2, "Gfx2");

    appOptions->SetBool("iPhone_autodetectScreenScaleFactor", true);

    appOptions->SetInt32("fullscreen", 0);

#else
    KeyedArchive* appOptions = new KeyedArchive();

    appOptions->SetInt32("width", 1024);
    appOptions->SetInt32("height", 768);

    appOptions->SetInt32("fullscreen", 0);
    appOptions->SetInt32("bpp", 32);
    appOptions->SetString(String("title"), String("Performance Tests"));

    DAVA::VirtualCoordinatesSystem::Instance()->SetVirtualScreenSize(1024, 768);
    DAVA::VirtualCoordinatesSystem::Instance()->RegisterAvailableResourceSize(1024, 768, "Gfx");
#endif 

#if defined(__DAVAENGINE_WIN_UAP__)
    appOptions->SetInt32("renderer", rhi::RHI_DX11);
#else
    appOptions->SetInt32("renderer", rhi::RHI_GLES2);
#endif

    GameCore* core = new GameCore();
    DAVA::Core::SetApplicationCore(core);
    DAVA::Core::Instance()->SetOptions(appOptions);
}

void FrameworkWillTerminate()
{
}
