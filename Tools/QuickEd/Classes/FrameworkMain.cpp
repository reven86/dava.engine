#include "DAVAEngine.h"
#include "GameCore.h"
#include "Version.h"
#include "Platform/DPIHelper.h"

using namespace DAVA;

void FrameworkDidLaunched()
{
    KeyedArchive* appOptions = new KeyedArchive();

    appOptions->SetInt32("fullscreen", 0);
    appOptions->SetInt32("bpp", 32);
    appOptions->SetBool("trackFont", true);

    appOptions->SetInt32("renderer", rhi::RHI_GLES2);
    appOptions->SetString("title", DAVA::Format("DAVA Framework - QuickEd | %s-%s [%u bit]", DAVAENGINE_VERSION, APPLICATION_BUILD_VERSION,
                                                static_cast<DAVA::uint32>(sizeof(DAVA::pointer_size) * 8)));

    Size2i screenSize = DPIHelper::GetScreenSize();
    appOptions->SetInt32("width", screenSize.dx);
    appOptions->SetInt32("height", screenSize.dy);
    VirtualCoordinatesSystem::Instance()->SetVirtualScreenSize(screenSize.dx, screenSize.dy);
    VirtualCoordinatesSystem::Instance()->RegisterAvailableResourceSize(screenSize.dx, screenSize.dy, "Gfx");

    Core::Instance()->SetOptions(appOptions);
    VirtualCoordinatesSystem::Instance()->EnableReloadResourceOnResize(false);

    GameCore* core = new GameCore();
    Core::SetApplicationCore(core);

    DynamicBufferAllocator::SetPageSize(1024 * 512);
}

void FrameworkWillTerminate()
{
}
