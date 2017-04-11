#include "UIViewerApp.h"
#include "UIScreens/UIViewScreen.h"

#include <Engine/Engine.h>
#include <Engine/Window.h>
#include <Render/RHI/rhi_Public.h>
#include <Utils/StringFormat.h>
#include <UI/UIScreenManager.h>

UIViewerApp::UIViewerApp(DAVA::Engine& engine_)
    : engine(engine_)
{
    engine.gameLoopStarted.Connect(this, &UIViewerApp::OnAppStarted);
    engine.windowCreated.Connect(this, &UIViewerApp::OnWindowCreated);
    engine.gameLoopStopped.Connect(this, &UIViewerApp::OnAppFinished);
    engine.suspended.Connect(this, &UIViewerApp::OnSuspend);
    engine.resumed.Connect(this, &UIViewerApp::OnResume);
    engine.beginFrame.Connect(this, &UIViewerApp::BeginFrame);
    engine.endFrame.Connect(this, &UIViewerApp::EndFrame);
}

void UIViewerApp::OnAppStarted()
{
}

void UIViewerApp::OnWindowCreated(DAVA::Window* w)
{
    using namespace DAVA;

    engine.PrimaryWindow()->draw.Connect(this, &UIViewerApp::Draw);

    const Size2i& physicalSize = UIControlSystem::Instance()->vcs->GetPhysicalScreenSize();
    float32 screenAspect = static_cast<float32>(physicalSize.dx) / static_cast<float32>(physicalSize.dy);

    const Size2f windowSize = { 1024.f, 1024.f / screenAspect };

    DAVA::String title = DAVA::Format("DAVA Engine - UI Viewer | %s [%u bit]", DAVAENGINE_VERSION,
                                      static_cast<DAVA::uint32>(sizeof(DAVA::pointer_size) * 8));

    w->SetTitleAsync(title);

    w->SetSizeAsync(windowSize);
    w->SetVirtualSize(windowSize.dx, windowSize.dy);

    VirtualCoordinatesSystem* vcs = DAVA::UIControlSystem::Instance()->vcs;
    vcs->RegisterAvailableResourceSize(static_cast<int32>(windowSize.dx), static_cast<int32>(windowSize.dy), "Gfx");
    vcs->RegisterAvailableResourceSize(static_cast<int32>(windowSize.dx * 2.0f), static_cast<int32>(windowSize.dy * 2.0f), "Gfx2");

    Renderer::SetDesiredFPS(60);

    uiViewScreen = new UIViewScreen(w);
    UIScreenManager::Instance()->SetFirst(uiViewScreen->GetScreenID());
}

void UIViewerApp::OnAppFinished()
{
    SafeRelease(uiViewScreen);
}

void UIViewerApp::OnSuspend()
{
}

void UIViewerApp::OnResume()
{
}

void UIViewerApp::BeginFrame()
{
}

void UIViewerApp::Draw(DAVA::Window* /*window*/)
{
}

void UIViewerApp::EndFrame()
{
}

//////////////////////////////////////////////////////////////////////////////////////////////

DAVA::KeyedArchive* CreateOptions()
{
    DAVA::KeyedArchive* appOptions = new DAVA::KeyedArchive();

    appOptions->SetInt32("shader_const_buffer_size", 4 * 1024 * 1024);

    appOptions->SetInt32("max_index_buffer_count", 3 * 1024);
    appOptions->SetInt32("max_vertex_buffer_count", 3 * 1024);
    appOptions->SetInt32("max_const_buffer_count", 16 * 1024);
    appOptions->SetInt32("max_texture_count", 2048);
    appOptions->SetInt32("max_texture_set_count", 2048);
    appOptions->SetInt32("max_sampler_state_count", 128);
    appOptions->SetInt32("max_pipeline_state_count", 1024);
    appOptions->SetInt32("max_depthstencil_state_count", 256);
    appOptions->SetInt32("max_render_pass_count", 64);
    appOptions->SetInt32("max_command_buffer_count", 64);
    appOptions->SetInt32("max_packet_list_count", 64);

#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    appOptions->SetInt32("renderer", rhi::RHI_GLES2);
    //appOptions->SetInt32("renderer", rhi::RHI_METAL);
    appOptions->SetInt32("rhi_threaded_frame_count", 2);
    appOptions->SetBool("iPhone_autodetectScreenScaleFactor", true);

#elif defined(__DAVAENGINE_WIN_UAP__)
    appOptions->SetInt32("bpp", 32);
    appOptions->SetInt32("renderer", rhi::RHI_DX11);
    appOptions->SetInt32("rhi_threaded_frame_count", 2);

#else
#if defined(__DAVAENGINE_WIN32__)
    //appOptions->SetInt32("renderer", rhi::RHI_DX9);
    //appOptions->SetInt32("renderer", rhi::RHI_DX9);
    appOptions->SetInt32("renderer", rhi::RHI_GLES2);
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
    DAVA::Vector<DAVA::String> modules =
    {
      "JobManager",
      "NetCore",
      "LocalizationSystem",
      "SoundSystem",
      "DownloadManager",
    };
    DAVA::Engine e;
    e.Init(DAVA::eEngineRunMode::GUI_STANDALONE, modules, CreateOptions());

    UIViewerApp app(e);
    return e.Run();
}
