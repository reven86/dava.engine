#include "CommandLine/Private/REConsoleModuleCommon.h"
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
    for (uint32 i = 0, count = levels.size(); i < count; ++i)
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

REConsoleModuleCommon::REConsoleModuleCommon(const DAVA::String& moduleName)
    : options(moduleName)
{
    options.AddOption("-log", DAVA::VariantType(DAVA::String("i")), "Set up the level of logging: e - error, w - warning, i - info, d - debug, f - framework. Info by default");
    options.AddOption("-h", DAVA::VariantType(false), "Help for command");
    options.AddOption("-teamcity", DAVA::VariantType(false), "Extra output in teamcity format");
}

void REConsoleModuleCommon::PostInit()
{
    DAVA::String logLevel = options.GetOption("-log").AsString();
    REConsoleModuleCommonDetail::SetupLogger(logLevel);

    bool useTeamcity = options.GetOption("-teamcity").AsBool();
    if (useTeamcity)
    {
        DAVA::Logger::AddCustomOutput(new DAVA::TeamcityOutput());
    }

    isInitialized = PostInitInternal();
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
    //base implementation is empty
}
