#include "SceneViewerApp.h"
#include "UIScreens/ViewSceneScreen.h"
#include "UIScreens/PerformanceResultsScreen.h"
#include "Quality/QualityPreferences.h"

#include <Engine/Engine.h>
#include <Engine/Window.h>

#include <Render/RHI/rhi_Public.h>
#include <Render/RHI/dbg_Draw.h>
#include <Render/RHI/Common/dbg_StatSet.h>
#include <Render/RHI/Common/rhi_Private.h>
#include <Render/ShaderCache.h>
#include <Render/Material/FXCache.h>

SceneViewerApp::SceneViewerApp(DAVA::Engine& engine)
    : data({ engine })
{
    engine.gameLoopStarted.Connect(this, &SceneViewerApp::OnAppStarted);
    engine.windowCreated.Connect(this, &SceneViewerApp::OnWindowCreated);
    engine.gameLoopStopped.Connect(this, &SceneViewerApp::OnAppFinished);
    engine.suspended.Connect(this, &SceneViewerApp::OnSuspend);
    engine.resumed.Connect(this, &SceneViewerApp::OnResume);
    engine.beginFrame.Connect(this, &SceneViewerApp::BeginFrame);
    engine.endFrame.Connect(this, &SceneViewerApp::EndFrame);

    DAVA::QualitySettingsSystem::Instance()->SetKeepUnusedEntities(true);
    DAVA::QualitySettingsSystem::Instance()->SetMetalPreview(true);
    DAVA::QualitySettingsSystem::Instance()->SetRuntimeQualitySwitching(true);

    QualityPreferences::LoadFromSettings(data.settings);
    data.scenePath = data.settings.GetLastOpenedScenePath();
}

void SceneViewerApp::OnAppStarted()
{
}

void SceneViewerApp::OnWindowCreated(DAVA::Window* w)
{
    using namespace DAVA;

    data.engine.PrimaryWindow()->draw.Connect(this, &SceneViewerApp::Draw);

    const Size2i& physicalSize = UIControlSystem::Instance()->vcs->GetPhysicalScreenSize();
    data.screenAspect = static_cast<float32>(physicalSize.dx) / static_cast<float32>(physicalSize.dy);

    const Size2f windowSize = { 1024.f, 1024.f / data.screenAspect };

    DAVA::String title = DAVA::Format("DAVA Engine - Scene Viewer | %s [%u bit]", DAVAENGINE_VERSION,
                                      static_cast<DAVA::uint32>(sizeof(DAVA::pointer_size) * 8));

    w->SetTitleAsync(title);

    w->SetSizeAsync(windowSize);
    w->SetVirtualSize(windowSize.dx, windowSize.dy);

    VirtualCoordinatesSystem* vcs = DAVA::UIControlSystem::Instance()->vcs;
    vcs->RegisterAvailableResourceSize(static_cast<int32>(windowSize.dx), static_cast<int32>(windowSize.dy), "Gfx");

    Renderer::SetDesiredFPS(60);
    HashMap<FastName, int32> flags;
    //flags[FastName("VERTEX_LIT")] = 1;
    //flags[FastName("PIXEL_LIT")] = 1;
    //flags[FastName("FRESNEL_TO_ALPHA")] = 1;
    //flags[FastName("REAL_REFLECTION")] = 1;
    //ShaderDescriptorCache::GetShaderDescriptor(FastName("~res:/Materials/Shaders/Default/water"), flags);

    //     flags[FastName("PIXEL_LIT")] = 1;
    //     flags[FastName("NORMALIZED_BLINN_PHONG")] = 1;
    //     flags[FastName("VERTEX_FOG")] = 1;
    //     flags[FastName("FOG_LINEAR")] = 0;
    //     flags[FastName("FOG_HALFSPACE")] = 1;
    //     flags[FastName("FOG_HALFSPACE_LINEAR")] = 1;
    //     flags[FastName("FOG_ATMOSPHERE")] = 1;
    //     flags[FastName("SKINNING")] = 1;
    //     flags[FastName("MATERIAL_TEXTURE")] = 1;
    //     ShaderDescriptorCache::GetShaderDescriptor(FastName("~res:/Materials/Shaders/Default/materials"), flags);

    //flags[FastName("SKINNING")] = 1;
    //const FXDescriptor& res = FXCache::GetFXDescriptor(FastName("~res:/Materials/Silhouette.material"), flags);

    viewSceneScreen = new ViewSceneScreen(data);
#ifdef WITH_SCENE_PERFORMANCE_TESTS
    performanceResultsScreen = new PerformanceResultsScreen(data);
#endif

    //SetScenePath( "~doc:/GB/Cromwell-test.sc2" );
    //    SetScenePath("~doc:/effect.sc2");
    //    SetScenePath("~doc:/karelia/karelia.sc2");
    //SetScenePath("~res:/3d/Maps/05_amigosville_am/05_amigosville_am.sc2");
    //    SetScenePath("~doc:/scene_viewer/test_box/box.sc2");
    //SetScenePath("~res:/amigosville/amigosville.sc2");
    //      SetScenePath("~doc:/fort/fort.sc2");
    //      SetScenePath("~doc:/USSR/T-62A_crash.sc2");
    //      SetScenePath("~doc:/amigosville/amigosville2.sc2");
    //      SetScenePath("~doc:/amigosville/amigosville5.sc2");
    //      SetScenePath("~doc:/amigosville/amigosville4.sc2");
    //      SetScenePath("~doc:/aaaa.sc2");
    //    SetScenePath("~doc:/karelia/karelia_landscape.sc2");
    //    SetScenePath("~doc:/karelia/gates_test.sc2");
    //SetScenePath("~doc:/karelia/objects/k_s01.sc2");
    UIScreenManager::Instance()->SetFirst(viewSceneScreen->GetScreenID());
    //UIScreenManager::Instance()->SetFirst(selectSceneScreen->GetScreenID());

    DbgDraw::EnsureInited();
}

void SceneViewerApp::OnAppFinished()
{
    data.scene.reset();

    DAVA::DbgDraw::Uninitialize();

    SafeRelease(viewSceneScreen);
#ifdef WITH_SCENE_PERFORMANCE_TESTS
    SafeRelease(performanceResultsScreen);
#endif
}

void SceneViewerApp::OnSuspend()
{
}

void SceneViewerApp::OnResume()
{
}

void SceneViewerApp::BeginFrame()
{
}

void SceneViewerApp::Draw(DAVA::Window* /*window*/)
{
#if 0
    rhi::RenderPassConfig pass_desc;

    pass_desc.colorBuffer[0].loadAction = rhi::LOADACTION_NONE;
    pass_desc.colorBuffer[0].storeAction = rhi::STOREACTION_NONE;
    pass_desc.depthStencilBuffer.loadAction = rhi::LOADACTION_NONE;
    pass_desc.depthStencilBuffer.storeAction = rhi::STOREACTION_NONE;
    pass_desc.priority = -10000;
    pass_desc.viewport.width = Renderer::GetFramebufferWidth();
    pass_desc.viewport.height = Renderer::GetFramebufferHeight();

    rhi::HPacketList pl;
    rhi::HRenderPass pass = rhi::AllocateRenderPass(pass_desc, 1, &pl);

    rhi::BeginRenderPass(pass);
    rhi::BeginPacketList(pl);
    DbgDraw::FlushBatched(pl, Matrix4(), Matrix4());
    rhi::EndPacketList(pl);
    rhi::EndRenderPass(pass);
#endif
}

void SceneViewerApp::EndFrame()
{
#if 0
    // stats must be obtained and reset AFTER frame is finished (and Present called)

    const char* backend = "";
    const uint32 color1 = rhi::NativeColorRGBA(0.9f, 0.9f, 1.0f, 1);
    const uint32 color2 = rhi::NativeColorRGBA(0.8f, 0.8f, 0.8f, 1);
    const int x0 = 10;
    const int y0 = 40;

    switch (rhi::HostApi())
    {
    case rhi::RHI_DX9:
        backend = "DX9";
        break;
    case rhi::RHI_DX11:
        backend = "DX11";
        break;
    case rhi::RHI_GLES2:
        backend = "GLES2";
        break;
    case rhi::RHI_METAL:
        backend = "Metal";
        break;
    }

    DbgDraw::SetScreenSize(VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize().dx, VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize().dy);
    //    DbgDraw::FilledRect2D( x0, y0, x0+13*DbgDraw::NormalCharW, y0+6*(DbgDraw::NormalCharH+1), rhi::NativeColorRGBA(0,0,0,0.4f) );
    DbgDraw::Text2D(x0, y0, rhi::NativeColorRGBA(1, 1, 1, 1), "RHI stats (%s)", backend);
    DbgDraw::Text2D(x0, y0 + 1 * (DbgDraw::NormalCharH + 1), color1, "  DIP     %u", StatSet::StatValue(rhi::stat_DIP));
    DbgDraw::Text2D(x0, y0 + 2 * (DbgDraw::NormalCharH + 1), color2, "  DP      %u", StatSet::StatValue(rhi::stat_DP));
    DbgDraw::Text2D(x0, y0 + 3 * (DbgDraw::NormalCharH + 1), color1, "  SET-PS  %u", StatSet::StatValue(rhi::stat_SET_PS));
    DbgDraw::Text2D(x0, y0 + 4 * (DbgDraw::NormalCharH + 1), color2, "  SET-TEX %u", StatSet::StatValue(rhi::stat_SET_TEX));
    DbgDraw::Text2D(x0, y0 + 5 * (DbgDraw::NormalCharH + 1), color1, "  SET-CB  %u", StatSet::StatValue(rhi::stat_SET_CB));

    StatSet::ResetAll();
#endif
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

    //appOptions->SetInt("fullscreen.width",	1280);
    //appOptions->SetInt("fullscreen.height", 800);

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

    SceneViewerApp app(e);
    return e.Run();
}
