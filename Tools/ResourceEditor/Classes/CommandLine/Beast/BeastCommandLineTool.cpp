#include "CommandLine/Beast/BeastCommandLineTool.h"
#include "CommandLine/SceneUtils/SceneUtils.h"

#include "Scene/SceneEditor2.h"
#include "Commands2/BeastAction.h"

#include "CommandLine/OptionName.h"

using namespace DAVA;

#if defined(__DAVAENGINE_BEAST__)

BeastCommandLineTool::BeastCommandLineTool()
    : CommandLineTool("-beast")
{
    options.AddOption(OptionName::File, VariantType(String("")), "Full pathname of scene for beasting");
    options.AddOption(OptionName::Output, VariantType(String("")), "Full path for output folder for beasting");
    options.AddOption(OptionName::QualityConfig, VariantType(String("")), "Full path for quality.yaml file");
}

void BeastCommandLineTool::ConvertOptionsToParamsInternal()
{
    scenePathname = options.GetOption(OptionName::File).AsString();
    outputPath = options.GetOption(OptionName::Output).AsString();
    qualityConfigPath = options.GetOption(OptionName::QualityConfig).AsString();
}

bool BeastCommandLineTool::InitializeInternal()
{
    if (scenePathname.IsEmpty() || !scenePathname.IsEqualToExtension(".sc2"))
    {
        Logger::Error("Scene was not selected");
        return false;
    }

    if (outputPath.IsEmpty())
    {
        Logger::Error("Out folder was not selected");
        return false;
    }
    outputPath.MakeDirectoryPathname();

    return true;
}

void BeastCommandLineTool::ProcessInternal()
{
    ScopedPtr<SceneEditor2> scene(new SceneEditor2());
    if (scene->LoadScene(scenePathname) == SceneFileV2::eError::ERROR_NO_ERROR)
    {
        scene->Update(0.1f);
        scene->Exec(Command2::Create<BeastAction>(scene, outputPath, BeastProxy::MODE_LIGHTMAPS, nullptr));
        scene->SaveScene();
    }
    RenderObjectsFlusher::Flush();
}

DAVA::FilePath BeastCommandLineTool::GetQualityConfigPath() const
{
    if (qualityConfigPath.IsEmpty())
    {
        return CreateQualityConfigPath(scenePathname);
    }

    return qualityConfigPath;
}


#endif //#if defined (__DAVAENGINE_BEAST__)
