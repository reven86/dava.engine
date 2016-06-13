#include "Base/BaseTypes.h"
#include "UI/UIScreenManager.h"
#include "CommandLine/CommandLineParser.h"
#include "Utils/Utils.h"
#include "UnitTests/UnitTests.h"

#include "Infrastructure/GameCore.h"

#if defined(__DAVAENGINE_WIN_UAP__)
#include "Network/NetConfig.h"
#include "Platform/TemplateWin32/UAPNetworkHelper.h"
#endif

using namespace DAVA;

namespace
{
// List of semicolon separated names specifying which test classes should run
String runOnlyTheseTestClasses = "";
// List of semicolon separated names specifying which test classes shouldn't run. This list takes precedence over runOnlyTheseTests
String disableTheseTestClasses = "";

bool teamcityOutputEnabled = true; // Flag whether to enable TeamCity output
bool teamcityCaptureStdout = false; // Flag whether to set TeamCity option 'captureStandardOutput=true'
}

void GameCore::ProcessCommandLine()
{
    CommandLineParser* cmdline = CommandLineParser::Instance();
    if (cmdline->CommandIsFound("-only_test"))
    {
        runOnlyTheseTestClasses = cmdline->GetCommandParam("-only_test");
    }
    if (cmdline->CommandIsFound("-disable_test"))
    {
        disableTheseTestClasses = cmdline->GetCommandParam("-disable_test");
    }
    if (cmdline->CommandIsFound("-noteamcity"))
    {
        teamcityOutputEnabled = false;
    }
    if (cmdline->CommandIsFound("-teamcity_capture_stdout"))
    {
        teamcityCaptureStdout = true;
    }
}

void GameCore::OnAppStarted()
{
    ProcessCommandLine();
#if defined(__DAVAENGINE_WIN_UAP__)
    InitNetwork();
#endif

    if (teamcityOutputEnabled)
    {
        teamCityOutput.SetCaptureStdoutFlag(teamcityCaptureStdout);
        Logger::Instance()->AddCustomOutput(&teamCityOutput);
    }

    UnitTests::TestCore::Instance()->Init(MakeFunction(this, &GameCore::OnTestClassStarted),
                                          MakeFunction(this, &GameCore::OnTestClassFinished),
                                          MakeFunction(this, &GameCore::OnTestStarted),
                                          MakeFunction(this, &GameCore::OnTestFinished),
                                          MakeFunction(this, &GameCore::OnTestFailed),
                                          MakeFunction(this, &GameCore::OnTestClassDisabled));
    if (!runOnlyTheseTestClasses.empty())
    {
        UnitTests::TestCore::Instance()->RunOnlyTheseTestClasses(runOnlyTheseTestClasses);
    }
    if (!disableTheseTestClasses.empty())
    {
        UnitTests::TestCore::Instance()->DisableTheseTestClasses(disableTheseTestClasses);
    }

    if (!UnitTests::TestCore::Instance()->HasTestClasses())
    {
        Logger::Error("%s", "There are no test classes");
        Core::Instance()->Quit();
    }
}

void GameCore::OnAppFinished()
{
    if (teamcityOutputEnabled)
    {
        DAVA::Logger::Instance()->RemoveCustomOutput(&teamCityOutput);
    }

#if defined(__DAVAENGINE_WIN_UAP__)
    UnInitNetwork();
#endif
}

void GameCore::OnSuspend()
{
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    ApplicationCore::OnSuspend();
#endif
}

void GameCore::OnResume()
{
    ApplicationCore::OnResume();
}

#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
void GameCore::OnForeground()
{
    ApplicationCore::OnForeground();
}
#endif //#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)

void GameCore::Update(float32 timeElapsed)
{
    ProcessTests(timeElapsed);
    ApplicationCore::Update(timeElapsed);
}

void GameCore::OnError()
{
    DavaDebugBreak();
}

void GameCore::OnTestClassStarted(const DAVA::String& testClassName)
{
    Logger::Info("%s", TeamcityTestsOutput::FormatTestClassStarted(testClassName).c_str());
}

void GameCore::OnTestClassFinished(const DAVA::String& testClassName)
{
    Logger::Info("%s", TeamcityTestsOutput::FormatTestClassFinished(testClassName).c_str());
}

void GameCore::OnTestClassDisabled(const DAVA::String& testClassName)
{
    Logger::Info("%s", TeamcityTestsOutput::FormatTestClassDisabled(testClassName).c_str());
}

void GameCore::OnTestStarted(const DAVA::String& testClassName, const DAVA::String& testName)
{
    Logger::Info("%s", TeamcityTestsOutput::FormatTestStarted(testClassName, testName).c_str());
}

void GameCore::OnTestFinished(const DAVA::String& testClassName, const DAVA::String& testName)
{
    Logger::Info("%s", TeamcityTestsOutput::FormatTestFinished(testClassName, testName).c_str());
}

void GameCore::OnTestFailed(const String& testClassName, const String& testName, const String& condition, const char* filename, int lineno, const String& userMessage)
{
    OnError();

    String errorString;
    if (userMessage.empty())
    {
        errorString = Format("%s:%d: %s", filename, lineno, testName.c_str());
    }
    else
    {
        errorString = Format("%s:%d: %s (%s)", filename, lineno, testName.c_str(), userMessage.c_str());
    }
    Logger::Error("%s", TeamcityTestsOutput::FormatTestFailed(testClassName, testName, condition, errorString).c_str());
}

void GameCore::ProcessTests(float32 timeElapsed)
{
    if (!UnitTests::TestCore::Instance()->ProcessTests(timeElapsed))
    {
        // Output test coverage for sample
        Map<String, Vector<String>> map = UnitTests::TestCore::Instance()->GetTestCoverage();
        Logger::Info("Test coverage");
        for (const auto& x : map)
        {
            Logger::Info("  %s:", x.first.c_str());
            const Vector<String>& v = x.second;
            for (const auto& s : v)
            {
                Logger::Info("        %s", s.c_str());
            }
        }
        FinishTests();
    }
}

void GameCore::FinishTests()
{
    // Inform teamcity script we just finished all tests
    Logger::Debug("Finish all tests.");
    Core::Instance()->Quit();
}

#if defined(__DAVAENGINE_WIN_UAP__)
void GameCore::InitNetwork()
{
    using namespace Net;

    auto loggerCreate = [this](uint32 serviceId, void*) -> IChannelListener* {
        if (!loggerInUse)
        {
            loggerInUse = true;
            return &netLogger;
        }
        return nullptr;
    };

    NetCore::Instance()->RegisterService(
    NetCore::SERVICE_LOG,
    loggerCreate,
    [this](IChannelListener* obj, void*) -> void { loggerInUse = false; });

    eNetworkRole role = UAPNetworkHelper::GetCurrentNetworkRole();
    Net::Endpoint endpoint = UAPNetworkHelper::GetCurrentEndPoint();

    NetConfig config(role);
    config.AddTransport(TRANSPORT_TCP, endpoint);
    config.AddService(NetCore::SERVICE_LOG);

    netController = NetCore::Instance()->CreateController(config, nullptr);

    flusher.reset(new LogFlusher(&netLogger));
}

void GameCore::UnInitNetwork()
{
    netLogger.Uninstall();
    flusher->FlushLogs();
    flusher.reset();

    Net::NetCore::Instance()->DestroyControllerBlocked(netController);
}

#if defined(__DAVAENGINE_WIN_UAP__)

GameCore::LogFlusher::LogFlusher(Net::NetLogger* logger)
    : netLogger(logger)
{
    Logger::AddCustomOutput(this);
}

GameCore::LogFlusher::~LogFlusher()
{
    Logger::RemoveCustomOutput(this);
}

void GameCore::LogFlusher::FlushLogs()
{
    while (netLogger->IsChannelOpen() && netLogger->GetMessageQueueSize() != 0)
    {
        Net::NetCore::Instance()->Poll();
    }
}

void GameCore::LogFlusher::Output(DAVA::Logger::eLogLevel, const DAVA::char8*)
{
    FlushLogs();
}

#endif

#endif