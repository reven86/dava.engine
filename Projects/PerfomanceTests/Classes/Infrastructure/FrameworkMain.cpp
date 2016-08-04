#include "DAVAEngine.h"
#include "GameCore.h"

using namespace DAVA;

void FrameworkDidLaunched()
{
    KeyedArchive* appOptions = new KeyedArchive();

    appOptions->SetInt32("rhi_threaded_frame_count", 2);
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
    
#define IOS_WIDTH 1024
#define IOS_HEIGHT 768

    appOptions->SetInt32("orientation", Core::SCREEN_ORIENTATION_LANDSCAPE_AUTOROTATE);

    appOptions->SetInt32("width", IOS_WIDTH);
    appOptions->SetInt32("height", IOS_HEIGHT);
    appOptions->SetInt32("renderer", rhi::ApiIsSupported(rhi::RHI_METAL) ? rhi::RHI_METAL : rhi::RHI_GLES2);

    DAVA::VirtualCoordinatesSystem::Instance()->SetVirtualScreenSize(IOS_WIDTH, IOS_HEIGHT);
    DAVA::VirtualCoordinatesSystem::Instance()->SetProportionsIsFixed(false);
    DAVA::VirtualCoordinatesSystem::Instance()->RegisterAvailableResourceSize(IOS_WIDTH, IOS_HEIGHT, "Gfx");
    DAVA::VirtualCoordinatesSystem::Instance()->RegisterAvailableResourceSize(IOS_WIDTH * 2, IOS_HEIGHT * 2, "Gfx2");

    appOptions->SetBool("iPhone_autodetectScreenScaleFactor", true);

    appOptions->SetInt32("fullscreen", 0);

#else

    appOptions->SetInt32("width", 1024);
    appOptions->SetInt32("height", 768);

    appOptions->SetInt32("fullscreen", 0);
    appOptions->SetInt32("bpp", 32);
    appOptions->SetString(String("title"), String("Performance Tests"));

    DAVA::VirtualCoordinatesSystem::Instance()->SetVirtualScreenSize(1024, 768);
    DAVA::VirtualCoordinatesSystem::Instance()->RegisterAvailableResourceSize(1024, 768, "Gfx");

#if defined(__DAVAENGINE_WIN_UAP__)
    appOptions->SetInt32("renderer", rhi::RHI_DX11);
#else
    appOptions->SetInt32("renderer", rhi::RHI_GLES2);
#endif

#endif

    GameCore* core = new GameCore();
    DAVA::Core::SetApplicationCore(core);
    DAVA::Core::Instance()->SetOptions(appOptions);
}

void FrameworkWillTerminate()
{
}
