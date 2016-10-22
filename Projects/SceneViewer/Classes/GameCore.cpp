#include "GameCore.h"
#include "SelectSceneScreen.h"
#include "ViewSceneScreen.h"

#include "Render/RHI/rhi_Public.h"
#include "Render/RHI/dbg_Draw.h"
#include "Render/RHI/Common/dbg_StatSet.h"
#include "Render/RHI/Common/rhi_Private.h"
#include "Render/ShaderCache.h"
#include "Render/Material/FXCache.h"

using namespace DAVA;

GameCore::GameCore()
{
    selectSceneScreen = NULL;
    viewSceneScreen = NULL;
}

GameCore::~GameCore()
{
}

void GameCore::OnAppStarted()
{
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

    selectSceneScreen = new SelectSceneScreen();
    viewSceneScreen = new ViewSceneScreen();

    //SetScenePath( "~doc:/GB/Cromwell-test.sc2" );
    //    SetScenePath("~doc:/effect.sc2");
    //    SetScenePath("~doc:/karelia/karelia.sc2");
    SetScenePath("~res:/3d/Maps/05_amigosville_am/05_amigosville_am.sc2");
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

    perfQuerySet = rhi::CreatePerfQuerySet(16);
    perfQuerySetFired = false;
}

void GameCore::OnAppFinished()
{
    DbgDraw::Uninitialize();

    SafeRelease(selectSceneScreen);
    SafeRelease(viewSceneScreen);
}

void GameCore::OnSuspend()
{
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    ApplicationCore::OnSuspend();
#endif
}

void GameCore::OnResume()
{
    ApplicationCore::OnResume();
}


#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
void GameCore::OnDeviceLocked()
{
    Core::Instance()->Quit();
}

void GameCore::OnBackground()
{
}

void GameCore::OnForeground()
{
    ApplicationCore::OnForeground();
}

#endif //#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)

void GameCore::BeginFrame()
{
    if (perfQuerySetFired)
    {
        bool ready = false;
        bool valid = false;
        rhi::GetPerfQuerySetStatus(perfQuerySet, &ready, &valid);

        if (ready && valid)
        {
            uint64 freq = 0;
            uint64 frame_t0, frame_t1;
            uint64 clear_t0, clear_t1;
            uint64 p2d_t0, p2d_t1;
            uint64 main_t0, main_t1;

            rhi::GetPerfQuerySetFreq(perfQuerySet, &freq);
            //            Logger::Info("perf-query:  freq= %u",uint32(freq));

            rhi::GetPerfQuerySetFrameTimestamps(perfQuerySet, &frame_t0, &frame_t1);

            rhi::GetPerfQuerySetTimestamp(perfQuerySet, PERFQUERY__CLEAR_PASS_T0, &clear_t0);
            rhi::GetPerfQuerySetTimestamp(perfQuerySet, PERFQUERY__CLEAR_PASS_T1, &clear_t1);

            rhi::GetPerfQuerySetTimestamp(perfQuerySet, PERFQUERY__2D_PASS_T0, &p2d_t0);
            rhi::GetPerfQuerySetTimestamp(perfQuerySet, PERFQUERY__2D_PASS_T1, &p2d_t1);

            rhi::GetPerfQuerySetTimestamp(perfQuerySet, PERFQUERY__MAIN_PASS_T0, &main_t0);
            rhi::GetPerfQuerySetTimestamp(perfQuerySet, PERFQUERY__MAIN_PASS_T1, &main_t1);

            Logger::Info("  GPU frame = %.3f ms", float(frame_t1 - frame_t0) / float(freq / 1000));
            Logger::Info("    clear : %.3f ms", float(clear_t1 - clear_t0) / float(freq / 1000));
            Logger::Info("    main  : %.3f ms", float(main_t1 - main_t0) / float(freq / 1000));
            Logger::Info("    2d    : %.3f ms", float(p2d_t1 - p2d_t0) / float(freq / 1000));

            perfQuerySetFired = false;
        }
    }

    if (!perfQuerySetFired)
    {
        rhi::ResetPerfQuerySet(perfQuerySet);
        rhi::SetFramePerfQuerySet(perfQuerySet);
        perfQuerySetFired = true;
    }

    ApplicationCore::BeginFrame();
}

void GameCore::EndFrame()
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
    ApplicationCore::EndFrame();
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
