#include "GameCore.h"
#include "OverdrawTest.h"
#include "OverdrawTestingScreen.h"

#include "Debug/DVAssertDefaultHandlers.h"

#include "Engine/Engine.h"
#include "Engine/Window.h"

#include "Render/RHI/rhi_Public.h"
#include "Render/RHI/dbg_Draw.h"
#include "Render/RHI/Common/dbg_StatSet.h"
#include "Render/RHI/Common/rhi_Private.h"
#include "Render/ShaderCache.h"
#include "Render/Material/FXCache.h"

using namespace DAVA;

const String GameCore::testerScenePath = "~res:/3d/Maps/TestingScene.sc2";

GameCore::GameCore(Engine& e)
    : engine(e)
{
    DVASSERT(instance == nullptr);
    instance = this;

    selectSceneScreen = NULL;
    viewSceneScreen = NULL;

    engine.gameLoopStarted.Connect(this, &GameCore::OnAppStarted);
    engine.windowCreated.Connect(this, &GameCore::OnWindowCreated);
    engine.gameLoopStopped.Connect(this, &GameCore::OnAppFinished);
}

void GameCore::OnAppStarted()
{
}

void GameCore::OnWindowCreated(DAVA::Window* w)
{
    w->SetTitleAsync("Overdraw Performance Tester");
    w->SetSizeAsync({ 1024, 768 });
    w->SetVirtualSize(1024, 768);

    Renderer::SetDesiredFPS(60);

    selectSceneScreen = new OverdrawTest();
    viewSceneScreen = new OverdrawTestingScreen();

    SetScenePath(testerScenePath);
    UIScreenManager::Instance()->SetFirst(selectSceneScreen->GetScreenID());

    DbgDraw::EnsureInited();
}

void GameCore::OnAppFinished()
{
    DbgDraw::Uninitialize();

    SafeRelease(selectSceneScreen);
    SafeRelease(viewSceneScreen);
}

GameCore* GameCore::instance = nullptr;

//////////////////////////////////////////////////////////////////////////////////////////////

KeyedArchive* CreateOptions()
{
    KeyedArchive* appOptions = new KeyedArchive();

#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    if (rhi::ApiIsSupported(rhi::RHI_METAL))
        appOptions->SetInt32("renderer", rhi::RHI_METAL);
    else if (rhi::ApiIsSupported(rhi::RHI_GLES2))
        appOptions->SetInt32("renderer", rhi::RHI_GLES2);

    appOptions->SetInt32("rhi_threaded_frame_count", 2);
    appOptions->SetBool("iPhone_autodetectScreenScaleFactor", true);

#elif defined(__DAVAENGINE_WIN_UAP__)
    appOptions->SetInt32("bpp", 32);
    appOptions->SetInt32("renderer", rhi::RHI_DX11);
    appOptions->SetInt32("rhi_threaded_frame_count", 2);

#else
#if defined(__DAVAENGINE_WIN32__)
    appOptions->SetInt32("renderer", rhi::RHI_DX11);
    appOptions->SetInt32("rhi_threaded_frame_count", 2);
#elif defined(__DAVAENGINE_MACOS__)
    appOptions->SetInt32("renderer", rhi::RHI_GLES2);
#endif

    appOptions->SetInt32("bpp", 32);
#endif

    return appOptions;
}

int DAVAMain(DAVA::Vector<DAVA::String> cmdline)
{
    Assert::SetupDefaultHandlers();

    Vector<String> modules =
    {
      "JobManager",
      "NetCore",
      "LocalizationSystem",
      "SoundSystem",
      "DownloadManager",
    };
    DAVA::Engine e;
    e.Init(eEngineRunMode::GUI_STANDALONE, modules, CreateOptions());

    GameCore core(e);
    return e.Run();
}
