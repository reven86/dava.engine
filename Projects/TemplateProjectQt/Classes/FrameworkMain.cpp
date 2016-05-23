#include "DAVAEngine.h"
#include "GameCore.h"

void FrameworkDidLaunched()
{
    DAVA::KeyedArchive* appOptions = new DAVA::KeyedArchive();

    appOptions->SetString("title", DAVA::Format("DAVA Framework - TemplateProjectQt | %s", DAVAENGINE_VERSION));
    appOptions->SetInt32("bpp", 32);

    GameCore* core = new GameCore();
    DAVA::Core::SetApplicationCore(core);
    DAVA::Core::Instance()->SetOptions(appOptions);
    DAVA::VirtualCoordinatesSystem::Instance()->EnableReloadResourceOnResize(false);

    SafeRelease(appOptions);
}

void FrameworkWillTerminate()
{
}
