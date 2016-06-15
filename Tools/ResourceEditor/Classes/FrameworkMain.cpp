#include "Core/Core.h"
#include "Core/PerformanceSettings.h"
#include "FileSystem/KeyedArchive.h"
#include "Render/RHI/rhi_Type.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#include "Qt/Scene/System/VisibilityCheckSystem/VisibilityCheckSystem.h"

#include "GameCore.h"
#include "Version.h"

void FrameworkDidLaunched()
{
    DAVA::KeyedArchive* appOptions = new DAVA::KeyedArchive();

    auto title = DAVA::Format("DAVA Framework - ResourceEditor | %s.%s [%u bit]", DAVAENGINE_VERSION, APPLICATION_BUILD_VERSION,
                              static_cast<DAVA::uint32>(sizeof(DAVA::pointer_size) * 8));
    appOptions->SetString("title", title);

    appOptions->SetInt32("fullscreen", 0);
    appOptions->SetInt32("bpp", 32);
    appOptions->SetInt32("renderer", rhi::RHI_GLES2);
    appOptions->SetInt32("width", 1024);
    appOptions->SetInt32("height", 768);

    appOptions->SetInt32("max_index_buffer_count", 16384);
    appOptions->SetInt32("max_vertex_buffer_count", 16384);
    appOptions->SetInt32("max_const_buffer_count", 32767);
    appOptions->SetInt32("max_texture_count", 2048);

    appOptions->SetInt32("shader_const_buffer_size", 256 * 1024 * 1024);

    GameCore* core = new GameCore();
    DAVA::Core::SetApplicationCore(core);
    DAVA::Core::Instance()->SetOptions(appOptions);
    DAVA::VirtualCoordinatesSystem::Instance()->EnableReloadResourceOnResize(false);
    DAVA::PerformanceSettings::Instance()->SetPsPerformanceMinFPS(5.0f);
    DAVA::PerformanceSettings::Instance()->SetPsPerformanceMaxFPS(10.0f);

    SafeRelease(appOptions);
}

void FrameworkWillTerminate()
{
    VisibilityCheckSystem::ReleaseCubemapRenderTargets();
}
