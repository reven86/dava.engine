#include "CommandLine/Private/SceneConsoleHelper.h"
#include "CommandLine/ProgramOptions.h"

namespace SceneConsoleHelperDetail
{
/*
 * Flush implementation
 * temporary (hopefully!) solution to clean-up RHI's objects
 * when there is no run/render loop in the application
 */
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

DAVA::FilePath CreateQualityPathname(const DAVA::FilePath& qualityPathname, const DAVA::FilePath& targetPathname)
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
}

bool SceneConsoleHelper::InitializeQualitySystem(const DAVA::ProgramOptions& options, const DAVA::FilePath& targetPathname)
{
    DAVA::FilePath qualityPathname = options.GetOption(OptionName::QualityConfig).AsString();
    qualityPathname = SceneConsoleHelperDetail::CreateQualityPathname(qualityPathname, targetPathname);
    if (qualityPathname.IsEmpty())
    {
        return false;
    }

    QualitySettingsSystem::Instance()->Load(qualityPathname);
    return true;
}

void SceneConsoleHelper::FlushRHI()
{
    SceneConsoleHelperDetail::Flush();
}
