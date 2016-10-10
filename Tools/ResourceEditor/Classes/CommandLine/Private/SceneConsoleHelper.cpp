#include "CommandLine/Private/SceneConsoleHelper.h"

/*
 * RenderObjectsFlusher implementation
 * temporary (hopefully!) solution to clean-up RHI's objects
 * when there is no run/render loop in the application
 */

namespace SceneConsoleHelperDetail
{
DAVA_DEPRECATED(void Flush())
{
    static const rhi::HTexture nullTexture;
    static const rhi::Viewport nullViewport(0, 0, 1, 1);

    auto currentFrame = rhi::GetCurrentFrameSyncObject();
    while (!rhi::SyncObjectSignaled(currentFrame))
    {
        Renderer::BeginFrame();
        RenderHelper::CreateClearPass(nullTexture, nullTexture, 0, DAVA::Color::Clear, nullViewport);
        Renderer::EndFrame();
    }
}
}

DAVA::FilePath SceneConsoleHelper::CreateQualityPathname(const DAVA::FilePath& qualityPathname, const DAVA::FilePath& targetPathname)
{
    if (qualityPathname.IsEmpty() == false)
    {
        return qualityPathname;
    }

    DAVA::String fullPath = targetPathname.GetAbsolutePathname();
    DAVA::String::size_type pos = fullPath.find("/Data");
    if (pos != DAVA::String::npos)
    {
        return (fullPath.substr(0, pos) + "/Data/Quality.yaml");
    }

    return DAVA::FilePath();
}

bool SceneConsoleHelper::InitializeRendering(const DAVA::FilePath& qualityPathname)
{
    QualitySettingsSystem::Instance()->Load(qualityPathname);
}

void SceneConsoleHelper::ReleaseRendering()
{
    SceneConsoleHelperDetail::Flush();
}
