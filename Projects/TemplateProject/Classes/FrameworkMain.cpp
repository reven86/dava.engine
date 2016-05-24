#include "DAVAEngine.h"
#include "GameCore.h"

using namespace DAVA;

void FrameworkDidLaunched();
void FrameworkWillTerminate();

void FrameworkDidLaunched()
{
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    KeyedArchive* appOptions = new KeyedArchive();
    appOptions->SetInt32("orientation", Core::SCREEN_ORIENTATION_LANDSCAPE_RIGHT);
    appOptions->SetInt32("renderer", Core::RENDERER_OPENGL_ES_2_0);

    DAVA::VirtualCoordinatesSystem::Instance()->SetVirtualScreenSize(960, 480);
    DAVA::VirtualCoordinatesSystem::Instance()->RegisterAvailableResourceSize(960, 480, "Gfx");

#else
    KeyedArchive* appOptions = new KeyedArchive();

    appOptions->SetInt32("width", 920);
    appOptions->SetInt32("height", 690);

    // 	appOptions->SetInt("fullscreen.width",	1280);
    // 	appOptions->SetInt("fullscreen.height", 800);

    appOptions->SetInt32("fullscreen", 0);
    appOptions->SetInt32("bpp", 32);

    DAVA::VirtualCoordinatesSystem::Instance()->SetVirtualScreenSize(920, 690);
    DAVA::VirtualCoordinatesSystem::Instance()->RegisterAvailableResourceSize(920, 690, "Gfx");
#endif

    GameCore* core = new GameCore();
    DAVA::Core::SetApplicationCore(core);
    DAVA::Core::Instance()->SetOptions(appOptions);
}

void FrameworkWillTerminate()
{
}
