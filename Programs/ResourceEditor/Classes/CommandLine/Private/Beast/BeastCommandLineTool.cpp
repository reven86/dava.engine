#include "CommandLine/BeastCommandLineTool.h"

#if defined(__DAVAENGINE_BEAST__)

#include "Base/ScopedPtr.h"
#include "Logger/Logger.h"
#include "Scene3D/Scene.h"

#include "Beast/BeastRunner.h"

#include "CommandLine/Private/OptionName.h"
#include "CommandLine/Private/SceneConsoleHelper.h"
#include "Utils/SceneUtils/SceneUtils.h"

#include "TArc/Utils/ModuleCollection.h"

BeastCommandLineTool::BeastCommandLineTool(const DAVA::Vector<DAVA::String>& commandLine)
    : CommandLineModule(commandLine, "-beast")
{
    options.AddOption(OptionName::File, DAVA::VariantType(DAVA::String("")), "Full pathname of scene for beasting");
    options.AddOption(OptionName::Output, DAVA::VariantType(DAVA::String("")), "Full path for output folder for beasting");
    options.AddOption(OptionName::QualityConfig, DAVA::VariantType(DAVA::String("")), "Full path for quality.yaml file");
}

bool BeastCommandLineTool::PostInitInternal()
{
    scenePathname = options.GetOption(OptionName::File).AsString();
    if (scenePathname.IsEmpty() || !scenePathname.IsEqualToExtension(".sc2"))
    {
        DAVA::Logger::Error("Scene was not selected");
        return false;
    }

    outputPathname = options.GetOption(OptionName::Output).AsString();
    if (outputPathname.IsEmpty())
    {
        DAVA::Logger::Error("Out folder was not selected");
        return false;
    }
    else
    {
        outputPathname.MakeDirectoryPathname();
    }

    bool qualityInitialized = SceneConsoleHelper::InitializeQualitySystem(options, scenePathname);
    if (!qualityInitialized)
    {
        DAVA::Logger::Error("Cannot create path to quality.yaml from %s", scenePathname.GetAbsolutePathname().c_str());
        return false;
    }

    scene = new DAVA::Scene();
    if (scene->LoadScene(scenePathname) == DAVA::SceneFileV2::eError::ERROR_NO_ERROR)
    {
        scene->Update(0.1f);
        beastRunner = new BeastRunner(scene, scenePathname, outputPathname, BeastProxy::MODE_LIGHTMAPS, nullptr);
        beastRunner->Start();
    }
    else
    {
        SafeRelease(scene);
        return false;
    }

    return true;
}

DAVA::TArc::ConsoleModule::eFrameResult BeastCommandLineTool::OnFrameInternal()
{
    if (scene != nullptr && beastRunner != nullptr)
    {
        bool finished = beastRunner->Process();
        if (finished == false)
        {
            return DAVA::TArc::ConsoleModule::eFrameResult::CONTINUE;
        }
    }

    return DAVA::TArc::ConsoleModule::eFrameResult::FINISHED;
}

void BeastCommandLineTool::BeforeDestroyedInternal()
{
    if (beastRunner)
    {
        beastRunner->Finish(false);
    }

    if (scene != nullptr)
    {
        scene->SaveScene(scenePathname, false);
        DAVA::SafeRelease(scene);
    }

    DAVA::SafeDelete(beastRunner);

    SceneConsoleHelper::FlushRHI();
}

void BeastCommandLineTool::ShowHelpInternal()
{
    CommandLineModule::ShowHelpInternal();

    DAVA::Logger::Info("Examples:");
    DAVA::Logger::Info("\t-beast -file /Users/SmokeTest/DataSource/3d/Maps/scene.sc2 -output /Users/SmokeTest/DataSource/3d/Maps/beast");
}

DECL_CONSOLE_MODULE(BeastCommandLineTool, "-beast");

#endif //#if defined (__DAVAENGINE_BEAST__)
