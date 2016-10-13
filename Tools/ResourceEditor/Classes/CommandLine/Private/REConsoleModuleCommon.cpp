#include "CommandLine/Private/REConsoleModuleCommon.h"
#include "FileSystem/FilePath.h"
#include "Logger/Logger.h"
#include "Logger/TeamcityOutput.h"

namespace REConsoleModuleCommonDetail
{
void SetupLogger(const DAVA::String& logLevelString)
{
    //        enum DAVA::Logger::eLogLevel
    //        {
    //            LEVEL_FRAMEWORK = 0,
    //            LEVEL_DEBUG,
    //            LEVEL_INFO,
    //            LEVEL_WARNING,
    //            LEVEL_ERROR,
    //        };

    DAVA::Vector<DAVA::String> levels =
    {
      "f", "d", "i", "w", "e"
    };

    DAVA::Logger::eLogLevel requestedLevel = DAVA::Logger::LEVEL_INFO;
    for (DAVA::uint32 i = 0, count = levels.size(); i < count; ++i)
    {
        if (levels[i] == logLevelString)
        {
            requestedLevel = static_cast<DAVA::Logger::eLogLevel>(i);
            break;
        }
    }
    DAVA::Logger::Instance()->SetLogLevel(requestedLevel);
}
}

REConsoleModuleCommon::REConsoleModuleCommon(const DAVA::Vector<DAVA::String>& commandLine_, const DAVA::String& moduleName)
    : options(moduleName)
    , commandLine(commandLine_)
{
    options.AddOption("-log", DAVA::VariantType(DAVA::String("i")), "Set up the level of logging: e - error, w - warning, i - info, d - debug, f - framework. Info is defualt value");

    options.AddOption("-logfile", DAVA::VariantType(DAVA::String("")), "Path to file for logger output");

    options.AddOption("-h", DAVA::VariantType(false), "Help for command");
    options.AddOption("-teamcity", DAVA::VariantType(false), "Enable extra output in teamcity format");
}

void REConsoleModuleCommon::PostInit()
{
    isInitialized = options.Parse(commandLine);
    if (isInitialized)
    {
        DAVA::String logLevel = options.GetOption("-log").AsString();
        REConsoleModuleCommonDetail::SetupLogger(logLevel);

        DAVA::FilePath logFile = options.GetOption("-logfile").AsString();
        if (logFile.IsEmpty() == false)
        {
            DAVA::Logger::Instance()->SetLogPathname(logFile);
        }

        bool useTeamcity = options.GetOption("-teamcity").AsBool();
        if (useTeamcity)
        {
            DAVA::Logger::AddCustomOutput(new DAVA::TeamcityOutput());
        }
        isInitialized = PostInitInternal();
    }
}

DAVA::TArc::ConsoleModule::eFrameResult REConsoleModuleCommon::OnFrame()
{
    bool showHelp = options.GetOption("-h").AsBool();
    if (showHelp || isInitialized == false)
    {
        ShowHelpInternal();
        return DAVA::TArc::ConsoleModule::eFrameResult::FINISHED;
    }
    else
    {
        return OnFrameInternal();
    }

    return DAVA::TArc::ConsoleModule::eFrameResult::FINISHED;
}

void REConsoleModuleCommon::BeforeDestroyed()
{
    BeforeDestroyedInternal();
}

bool REConsoleModuleCommon::PostInitInternal()
{
    //base implementation is empty
    return true;
}

DAVA::TArc::ConsoleModule::eFrameResult REConsoleModuleCommon::OnFrameInternal()
{
    //base implementation is empty
    return DAVA::TArc::ConsoleModule::eFrameResult::FINISHED;
}

void REConsoleModuleCommon::BeforeDestroyedInternal()
{
    //base implementation is empty
}

void REConsoleModuleCommon::ShowHelpInternal()
{
    DAVA::String usage = options.GetUsageString();
    DAVA::Logger::Info("\nDetails:\n%s", usage.c_str());
}
