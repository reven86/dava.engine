#include "DAVAEngine.h"
#include "GameCore.h"

void FrameworkDidLaunched()
{
    DAVA::KeyedArchive* appOptions = new DAVA::KeyedArchive();

    appOptions->SetInt32("fullscreen", 0);
    appOptions->SetBool("trackFont", true);

    appOptions->SetString("title", DAVA::Format("DAVA Framework - TemplateProjectQt | %s", DAVAENGINE_VERSION));
    appOptions->SetInt32("bpp", 32);
    appOptions->SetInt32("renderer", rhi::RHI_GLES2);

    GameCore* core = new GameCore();
    DAVA::Core::SetApplicationCore(core);
    DAVA::Core::Instance()->SetOptions(appOptions);
    DAVA::VirtualCoordinatesSystem::Instance()->EnableReloadResourceOnResize(false);

    SafeRelease(appOptions);

    DAVA::ScopedPtr<DAVA::UIScreen> davaUIScreen(new DAVA::UIScreen());
    davaUIScreen->GetBackground()->SetDrawType(DAVA::UIControlBackground::DRAW_FILL);
    davaUIScreen->GetBackground()->SetColor(DAVA::Color(1.f, 0.f, 0.f, 1.f));
    DAVA::UIScreenManager::Instance()->RegisterScreen(0, davaUIScreen);
    DAVA::UIScreenManager::Instance()->SetFirst(0);
}

void FrameworkWillTerminate()
{
}
