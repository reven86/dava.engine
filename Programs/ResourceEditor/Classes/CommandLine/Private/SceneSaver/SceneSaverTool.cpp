#include "CommandLine/SceneSaverTool.h"
#include "CommandLine/Private/OptionName.h"
#include "CommandLine/Private/SceneConsoleHelper.h"
#include "Utils/SceneSaver/SceneSaver.h"
#include "Logger/Logger.h"
#include "TArc/Utils/ModuleCollection.h"

SceneSaverTool::SceneSaverTool(const DAVA::Vector<DAVA::String>& commandLine)
    : CommandLineModule(commandLine, "-scenesaver")
{
    using namespace DAVA;

    options.AddOption(OptionName::Save, VariantType(false), "Saving scene from indir to outdir");
    options.AddOption(OptionName::Resave, VariantType(false), "Resave file into indir");
    options.AddOption(OptionName::Yaml, VariantType(false), "Target is *.yaml file");
    options.AddOption(OptionName::InDir, VariantType(String("")), "Path for Project/DataSource/3d/ folder");
    options.AddOption(OptionName::OutDir, VariantType(String("")), "Path for Project/Data/3d/ folder");
    options.AddOption(OptionName::ProcessFile, VariantType(String("")), "Filename from DataSource/3d/ for exporting");
    options.AddOption(OptionName::CopyConverted, VariantType(false), "Enables copying of converted image files");
    options.AddOption(OptionName::QualityConfig, VariantType(String("")), "Full path for quality.yaml file");
}

bool SceneSaverTool::PostInitInternal()
{
    using namespace DAVA;

    inFolder = options.GetOption(OptionName::InDir).AsString();
    if (inFolder.IsEmpty())
    {
        Logger::Error("Input folder was not selected");
        return false;
    }
    inFolder.MakeDirectoryPathname();

    outFolder = options.GetOption(OptionName::OutDir).AsString();
    filename = options.GetOption(OptionName::ProcessFile).AsString();
    copyConverted = options.GetOption(OptionName::CopyConverted).AsBool();

    if (options.GetOption(OptionName::Save).AsBool())
    {
        commandAction = ACTION_SAVE;
    }
    else if (options.GetOption(OptionName::Resave).AsBool())
    {
        if (options.GetOption(OptionName::Yaml).AsBool())
        {
            commandAction = ACTION_RESAVE_YAML;
        }
        else
        {
            commandAction = ACTION_RESAVE_SCENE;
        }
    }

    if (commandAction == ACTION_SAVE)
    {
        if (outFolder.IsEmpty())
        {
            Logger::Error("Output folder was not selected");
            return false;
        }
        outFolder.MakeDirectoryPathname();
    }
    else if (commandAction == ACTION_NONE)
    {
        Logger::Error("Wrong action was selected");
        return false;
    }

    if (filename.empty() && (commandAction != eAction::ACTION_RESAVE_YAML))
    {
        Logger::Error("Filename was not selected");
        return false;
    }

    if (commandAction != ACTION_RESAVE_YAML)
    {
        bool qualityInitialized = SceneConsoleHelper::InitializeQualitySystem(options, inFolder);
        if (!qualityInitialized)
        {
            DAVA::Logger::Error("Cannot create path to quality.yaml from %s", inFolder.GetAbsolutePathname().c_str());
            return false;
        }
    }

    return true;
}

DAVA::TArc::ConsoleModule::eFrameResult SceneSaverTool::OnFrameInternal()
{
    switch (commandAction)
    {
    case SceneSaverTool::eAction::ACTION_SAVE:
    {
        SceneSaver saver;
        saver.SetInFolder(inFolder);
        saver.SetOutFolder(outFolder);
        saver.EnableCopyConverted(copyConverted);
        saver.SaveFile(filename);

        break;
    }
    case SceneSaverTool::eAction::ACTION_RESAVE_SCENE:
    {
        SceneSaver saver;
        saver.SetInFolder(inFolder);
        saver.ResaveFile(filename);
        break;
    }
    case SceneSaverTool::eAction::ACTION_RESAVE_YAML:
    {
        SceneSaver saver;
        saver.ResaveYamlFilesRecursive(inFolder);
        break;
    }

    default:
        DAVA::Logger::Error("Unhandled action!");
        break;
    }

    return DAVA::TArc::ConsoleModule::eFrameResult::FINISHED;
}

void SceneSaverTool::BeforeDestroyedInternal()
{
    SceneConsoleHelper::FlushRHI();
}

void SceneSaverTool::ShowHelpInternal()
{
    CommandLineModule::ShowHelpInternal();

    DAVA::Logger::Info("Examples:");
    DAVA::Logger::Info("\t-scenesaver -save -indir /Users/SmokeTest/DataSource/3d/ -outdir /Users/NewProject/Data/3d/ -processfile Maps/scene.sc2 -qualitycfgpath Users/SmokeTest/Data/quality.yaml");
    DAVA::Logger::Info("\t-scenesaver -save -indir /Users/SmokeTest/DataSource/3d/ -outdir /Users/NewProject/Data/3d/ -processfile Maps/scene.sc2 -qualitycfgpath Users/SmokeTest/Data/quality.yaml -copyconverted");

    DAVA::Logger::Info("\t-scenesaver -resave -indir /Users/SmokeTest/DataSource/3d/ -processfile Maps/scene.sc2 -qualitycfgpath Users/SmokeTest/Data/quality.yaml");
    DAVA::Logger::Info("\t-scenesaver -resave -yaml -indir /Users/SmokeTest/Data/Configs/");
}

DECL_CONSOLE_MODULE(SceneSaverTool, "-scenesaver");
