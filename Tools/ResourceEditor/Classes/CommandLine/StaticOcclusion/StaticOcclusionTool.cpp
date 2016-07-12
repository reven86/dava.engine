#include "CommandLine/StaticOcclusion/StaticOcclusionTool.h"
#include "CommandLine/SceneUtils/SceneUtils.h"
#include "Scene/SceneEditor2.h"
#include "CommandLine/OptionName.h"

using namespace DAVA;

StaticOcclusionTool::StaticOcclusionTool()
    : CommandLineTool("-staticocclusion")
{
    options.AddOption(OptionName::Build, VariantType(false), "Enables build of static occlusion");
    options.AddOption(OptionName::ProcessFile, VariantType(String("")), "Full pathname to scene file *.sc2");
    options.AddOption(OptionName::QualityConfig, VariantType(String("")), "Full path for quality.yaml file");
}

void StaticOcclusionTool::ConvertOptionsToParamsInternal()
{
    if (options.GetOption(OptionName::Build).AsBool())
    {
        commandAction = ACTION_BUILD;
    }

    scenePathname = options.GetOption(OptionName::ProcessFile).AsString();
    qualityConfigPath = options.GetOption(OptionName::QualityConfig).AsString();
}

bool StaticOcclusionTool::InitializeInternal()
{
    if (commandAction == ACTION_NONE)
    {
        Logger::Error("Wrong action was selected");
        return false;
    }

    if (scenePathname.IsEmpty())
    {
        Logger::Error("Filename was not set");
        return false;
    }

    return true;
}

void StaticOcclusionTool::ProcessInternal()
{
    const rhi::HTexture nullTexture;
    const rhi::Viewport nullViewport(0, 0, 1, 1);

    DAVA::Logger::Info("Run static occlusion");
    if (commandAction == ACTION_BUILD)
    {
        ScopedPtr<SceneEditor2> scene(new SceneEditor2());
        if (scene->LoadScene(scenePathname) == SceneFileV2::eError::ERROR_NO_ERROR)
        {
            DAVA::Logger::Info("Before update");
            scene->Update(0.1f); // we need to call update to initialize (at least) QuadTree.
            DAVA::Logger::Info("After update");
            scene->staticOcclusionBuildSystem->Build();
            RenderObjectsFlusher::Flush();

            while (scene->staticOcclusionBuildSystem->IsInBuild())
            {
                Renderer::BeginFrame();
                RenderHelper::CreateClearPass(nullTexture, nullTexture, 0, DAVA::Color::Clear, nullViewport);
                scene->Update(0.1f);
                Renderer::EndFrame();
            }

            DAVA::Logger::Info("Before save Scene");
            scene->SaveScene();
            DAVA::Logger::Info("Save scene");
        }
        RenderObjectsFlusher::Flush();
    }
}

DAVA::FilePath StaticOcclusionTool::GetQualityConfigPath() const
{
    if (qualityConfigPath.IsEmpty())
    {
        return CreateQualityConfigPath(scenePathname);
    }

    return qualityConfigPath;
}
