#include "CommandLine/CommandLineModule.h"
#include "Engine/Engine.h"
#include "FileSystem/FilePath.h"
#include "Logger/Logger.h"
#include "Logger/TeamcityOutput.h"

namespace CommandLineModuleDetail
{
void SetupLogger(const DAVA::String& requestedLevelString)
{
    static const DAVA::UnorderedMap<DAVA::String, DAVA::Logger::eLogLevel> levels =
    {
      { "f", DAVA::Logger::eLogLevel::LEVEL_FRAMEWORK },
      { "d", DAVA::Logger::eLogLevel::LEVEL_DEBUG },
      { "i", DAVA::Logger::eLogLevel::LEVEL_INFO },
      { "w", DAVA::Logger::eLogLevel::LEVEL_WARNING },
      { "e", DAVA::Logger::eLogLevel::LEVEL_ERROR }
    };

    DAVA::Logger::eLogLevel requestedLevel = DAVA::Logger::LEVEL_INFO;
    const auto& found = levels.find(requestedLevelString);
    if (found != levels.end())
    {
        requestedLevel = found->second;
    }

    DAVA::GetEngineContext()->logger->SetLogLevel(requestedLevel);
}
}

CommandLineModule::CommandLineModule(const DAVA::Vector<DAVA::String>& commandLine_, const DAVA::String& moduleName)
    : commandLine(commandLine_)
    , options(moduleName)
{
    options.AddOption("-log", DAVA::VariantType(DAVA::String("i")), "Set up the level of logging: e - error, w - warning, i - info, d - debug, f - framework. Info is defualt value");

    options.AddOption("-logfile", DAVA::VariantType(DAVA::String("")), "Path to file for logger output");

    options.AddOption("-h", DAVA::VariantType(false), "Help for command");
    options.AddOption("-teamcity", DAVA::VariantType(false), "Enable extra output in teamcity format");
}

void CommandLineModule::PostInit()
{
    isInitialized = options.Parse(commandLine);
    if (isInitialized)
    {
        DAVA::String logLevel = options.GetOption("-log").AsString();
        CommandLineModuleDetail::SetupLogger(logLevel);

        DAVA::FilePath logFile = options.GetOption("-logfile").AsString();
        if (logFile.IsEmpty() == false)
        {
            DAVA::GetEngineContext()->logger->SetLogPathname(logFile);
        }

        bool useTeamcity = options.GetOption("-teamcity").AsBool();
        if (useTeamcity)
        {
            DAVA::Logger::AddCustomOutput(new DAVA::TeamcityOutput());
        }
        isInitialized = PostInitInternal();
    }

    if (!isInitialized)
    {
        result = DAVA::Result::RESULT_ERROR;
    }
}

DAVA::TArc::ConsoleModule::eFrameResult CommandLineModule::OnFrame()
{
    bool showHelp = options.GetOption("-h").AsBool();
    if (showHelp || isInitialized == false)
    {
        ShowHelpInternal();
        return DAVA::TArc::ConsoleModule::eFrameResult::FINISHED;
    }

    return OnFrameInternal();
}

void CommandLineModule::BeforeDestroyed()
{
    BeforeDestroyedInternal();
}

bool CommandLineModule::PostInitInternal()
{
    //base implementation is empty
    return true;
}

DAVA::TArc::ConsoleModule::eFrameResult CommandLineModule::OnFrameInternal()
{
    //base implementation is empty
    return DAVA::TArc::ConsoleModule::eFrameResult::FINISHED;
}

void CommandLineModule::BeforeDestroyedInternal()
{
    //base implementation is empty
}

void CommandLineModule::ShowHelpInternal()
{
    DAVA::String usage = options.GetUsageString();
    DAVA::Logger::Info("\nDetails:\n%s", usage.c_str());
}
