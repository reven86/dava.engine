#include "CommandLine/SceneSaver/SceneSaverTool.h"
#include "CommandLine/SceneSaver/SceneSaver.h"
#include "CommandLine/OptionName.h"

using namespace DAVA;

SceneSaverTool::SceneSaverTool()
    : CommandLineTool("-scenesaver")
{
    options.AddOption(OptionName::Save, VariantType(false), "Saving scene from indir to outdir");
    options.AddOption(OptionName::Resave, VariantType(false), "Resave file into indir");
    options.AddOption(OptionName::Yaml, VariantType(false), "Target is *.yaml file");
    options.AddOption(OptionName::InDir, VariantType(String("")), "Path for Project/DataSource/3d/ folder");
    options.AddOption(OptionName::OutDir, VariantType(String("")), "Path for Project/Data/3d/ folder");
    options.AddOption(OptionName::ProcessFile, VariantType(String("")), "Filename from DataSource/3d/ for exporting");
    options.AddOption(OptionName::CopyConverted, VariantType(false), "Enables copying of converted image files");
    options.AddOption(OptionName::QualityConfig, VariantType(String("")), "Full path for quality.yaml file");
}

void SceneSaverTool::ConvertOptionsToParamsInternal()
{
    inFolder = options.GetOption(OptionName::InDir).AsString();
    outFolder = options.GetOption(OptionName::OutDir).AsString();
    filename = options.GetOption(OptionName::ProcessFile).AsString();
    qualityConfigPath = options.GetOption(OptionName::QualityConfig).AsString();

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

    copyConverted = options.GetOption(OptionName::CopyConverted).AsBool();
}

bool SceneSaverTool::InitializeInternal()
{
    if (inFolder.IsEmpty())
    {
        Logger::Error("Input folder was not selected");
        return false;
    }
    inFolder.MakeDirectoryPathname();

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

    return true;
}

void SceneSaverTool::ProcessInternal()
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
        DVASSERT(false);
        break;
    }
}

DAVA::FilePath SceneSaverTool::GetQualityConfigPath() const
{
    if (qualityConfigPath.IsEmpty())
    {
        return CreateQualityConfigPath(inFolder);
    }

    return qualityConfigPath;
}
