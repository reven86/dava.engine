#include "CommandLine/CommandLineTool.h"
#include "CommandLine/CommandLineParser.h"
#include "CommandLine/OptionName.h"

#include "Scene3D/Systems/QualitySettingsSystem.h"
#include "Logger/TeamcityOutput.h"

#include "Project/ProjectManager.h"

using namespace DAVA;

CommandLineTool::CommandLineTool(const String& toolName)
    : options(toolName)
{
    options.AddOption("-v", VariantType(false), "Verbose output");
    options.AddOption("-h", VariantType(false), "Help for command");
    options.AddOption("-teamcity", VariantType(false), "Extra output in teamcity format");
    options.AddOption(OptionName::deprecated_forceClose, VariantType(false), "Deprecated. Need until unification of command line in RE.");
}

DAVA::String CommandLineTool::GetToolKey() const
{
    return options.GetCommand();
}

FilePath CommandLineTool::CreateQualityConfigPath(const FilePath& path) const
{
    FilePath projectPath = ProjectManager::CreateProjectPathFromPath(path);
    if (projectPath.IsEmpty())
        return projectPath;

    return (projectPath + "Data/Quality.yaml");
}

bool CommandLineTool::ParseCommandLine(int argc, char* argv[])
{
    return options.Parse(argc, argv);
}

bool CommandLineTool::Initialize()
{
    ConvertOptionsToParamsInternal();
    return InitializeInternal();
}

void CommandLineTool::PrintUsage() const
{
    printf("%s\n", options.GetUsageString().c_str());
}

void CommandLineTool::Process()
{
    const bool printUsage = options.GetOption("-h").AsBool();
    if (printUsage)
    {
        PrintUsage();
    }
    else
    {
        PrepareEnvironment();

        bool initialized = Initialize();
        if (initialized)
        {
            PrepareQualitySystem();
            ProcessInternal();
        }
        else
        {
            PrintUsage();
        }
    }
}

void CommandLineTool::PrepareEnvironment() const
{
    const bool verboseMode = options.GetOption("-v").AsBool();
    if (verboseMode)
    {
        CommandLineParser::Instance()->SetVerbose(true); //why we have this function?
        Logger::Instance()->SetLogLevel(Logger::LEVEL_DEBUG);
    }

    const bool useTeamcity = options.GetOption("-teamcity").AsBool();
    if (useTeamcity)
    {
        CommandLineParser::Instance()->SetUseTeamcityOutput(true); //why we have this ?
        Logger::AddCustomOutput(new TeamcityOutput());
    }
}

DAVA::FilePath CommandLineTool::GetQualityConfigPath() const
{
    return DAVA::FilePath();
};

void CommandLineTool::PrepareQualitySystem() const
{
    const FilePath qualitySettings = GetQualityConfigPath();
    if (!qualitySettings.IsEmpty())
    {
        QualitySettingsSystem::Instance()->Load(GetQualityConfigPath());
    }
}
