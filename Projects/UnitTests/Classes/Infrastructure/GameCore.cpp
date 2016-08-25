#include "Engine/EngineModule.h"
#include "Base/BaseTypes.h"
#include "UnitTests/UnitTests.h"
#include "CommandLine/CommandLineParser.h"
#include "FileSystem/KeyedArchive.h"
#include "Utils/Utils.h"

#include "Infrastructure/GameCore.h"

#if defined(__DAVAENGINE_WIN_UAP__)
#include "Network/NetConfig.h"
#include "Platform/TemplateWin32/UAPNetworkHelper.h"
#endif

using namespace DAVA;

namespace
{
// List of semicolon separated names specifying which test classes should run
String runOnlyTheseTestClasses = "AnyAnyFnTest";
// List of semicolon separated names specifying which test classes shouldn't run. This list takes precedence over runOnlyTheseTests
String disableTheseTestClasses = "";

bool teamcityOutputEnabled = true; // Flag whether to enable TeamCity output
bool teamcityCaptureStdout = false; // Flag whether to set TeamCity option 'captureStandardOutput=true'

const String TestCoverageFileName = "UnitTests.cover";
}

#if defined(__DAVAENGINE_COREV2__)

int GameMain(Vector<String> cmdline)
{
    KeyedArchive* appOptions = new KeyedArchive();
    appOptions->SetString("title", "UnitTests");
    appOptions->SetInt32("fullscreen", 0);
    appOptions->SetInt32("bpp", 32);
    appOptions->SetInt32("rhi_threaded_frame_count", 2);
    appOptions->SetInt32("width", 1024);
    appOptions->SetInt32("height", 768);
#if defined(__DAVAENGINE_QT__)
    appOptions->SetInt32("renderer", rhi::RHI_GLES2);
#elif defined(__DAVAENGINE_MACOS__)
    appOptions->SetInt32("renderer", rhi::RHI_GLES2);
#elif defined(__DAVAENGINE_IPHONE__)
    appOptions->SetInt32("renderer", rhi::RHI_GLES2);
#elif defined(__DAVAENGINE_WIN32__)
    appOptions->SetInt32("renderer", rhi::RHI_DX9);
#elif defined(__DAVAENGINE_WIN_UAP__)
    appOptions->SetInt32("renderer", rhi::RHI_DX11);
#elif defined(__DAVAENGINE_ANDROID__)
    appOptions->SetInt32("renderer", rhi::RHI_GLES2);
#endif

    Vector<String> modules = {
        "JobManager",
        "NetCore",
        "LocalizationSystem",
        "SoundSystem",
        "DownloadManager",
    };

    Engine e;
    e.SetOptions(appOptions);
    e.Init(eEngineRunMode::GUI_STANDALONE, modules);

    GameCore g(e);
    e.Run();
    return 0;
}

GameCore::GameCore(DAVA::Engine& e)
    : engine(e)
{
    engine.gameLoopStarted.Connect(this, &GameCore::OnAppStarted);
    engine.gameLoopStopped.Connect(this, &GameCore::OnAppFinished);
    engine.update.Connect(this, &GameCore::Update);
}

void GameCore::Update(float32 timeElapsed)
{
    ProcessTests(timeElapsed);
}

#else // __DAVAENGINE_COREV2__

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

#endif // !__DAVAENGINE_COREV2__

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
#if defined(__DAVAENGINE_COREV2__)
        engine.Quit();
#else
        Core::Instance()->Quit();
#endif
    }
    else
    {
#if defined(TEST_COVERAGE)
        RefPtr<File> covergeFile(File::Create(TestCoverageFileName, File::CREATE | File::WRITE));
        TEST_VERIFY(covergeFile);
        covergeFile->Flush();
#endif // __DAVAENGINE_MACOS__
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
            for (const String& s : v)
            {
                Logger::Info("        %s", s.c_str());
            }
        }

#if defined(TEST_COVERAGE)
        RefPtr<File> coverageFile(File::Create(TestCoverageFileName, File::APPEND | File::WRITE));
        DVASSERT(coverageFile);

        auto toJson = [&coverageFile](DAVA::String item) { coverageFile->Write(item.c_str(), item.size()); };

        toJson("{ \n    \"ProjectFolders\": \"" + DAVA::String(DAVA_FOLDERS) + "\",\n");

#if defined(DAVA_UNITY_FOLDER)
        toJson("    \"UnityFolder\": \"" + DAVA::String(DAVA_UNITY_FOLDER) + "\",\n");
#endif

        toJson("    \"Coverage\":  {\n");

        for (const auto& x : map)
        {
            toJson("         \"" + x.first + "\": \"");

            const Vector<String>& v = x.second;
            for (const String& s : v)
            {
                toJson(s + (&s != &*v.rbegin() ? " " : ""));
            }

            toJson(x.first != map.rbegin()->first ? "\",\n" : "\"\n");
        }

        toJson("     }\n");
        toJson("}\n");

#endif // TEST_COVERAGE

        FinishTests();
    }
}

void GameCore::FinishTests()
{
    // Inform teamcity script we just finished all tests
    Logger::Debug("Finish all tests.");
#if defined(__DAVAENGINE_COREV2__)
    engine.Quit();
#else
    Core::Instance()->Quit();
#endif
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
#endif // __DAVAENGINE_WIN_UAP__
