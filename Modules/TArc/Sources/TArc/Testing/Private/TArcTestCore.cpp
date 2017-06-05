#include "TArc/Testing/TArcTestCore.h"
#include "TArc/Testing/Private/DeadCodeTrick.h"

#include "Engine/Engine.h"

#include "UnitTests/TestCore.h"
#include "Utils/StringFormat.h"
#include "Logger/TeamCityTestsOutput.h"
#include "UnitTests/UnitTests.h"
#include "FileSystem/File.h"
#include "CommandLine/CommandLineParser.h"
#include "Engine/Qt/RenderWidget.h"

#include <QTimer>
#include <QApplication>
#include <gmock/gmock.h>

namespace DAVA
{
namespace TArcTestCoreDetail
{
// List of semicolon separated names specifying which test classes should run
String runOnlyTheseTestClasses = "";
// List of semicolon separated names specifying which test classes shouldn't run. This list takes precedence over runOnlyTheseTests
String disableTheseTestClasses = "";

bool teamcityOutputEnabled = true; // Flag whether to enable TeamCity output
bool teamcityCaptureStdout = false; // Flag whether to set TeamCity option 'captureStandardOutput=true'

const String TestCoverageFileName = "Tests.cover";
}

namespace TArc
{
TestCore::TestCore(Engine& e_)
    : e(e_)
{
    argv = e.GetCommandLineAsArgv();
    argc = static_cast<int>(argv.size());
    ::testing::InitGoogleMock(&argc, argv.data());
    ::testing::UnitTest::GetInstance()->listeners().Append(this);

    e.gameLoopStarted.Connect(this, &TestCore::OnAppStarted);
    e.gameLoopStopped.Connect(this, &TestCore::OnAppFinished);
    e.update.Connect(this, &TestCore::Update);

    bool result = AvoidTestsStriping();
    DAVA::Logger::Info("Avoid tests stripint result %d", result);
}

TestCore::~TestCore()
{
    ::testing::UnitTest::GetInstance()->listeners().Release(this);
}

void TestCore::OnAppStarted()
{
    ProcessCommandLine();

    if (TArcTestCoreDetail::teamcityOutputEnabled)
    {
        teamCityOutput.SetCaptureStdoutFlag(TArcTestCoreDetail::teamcityCaptureStdout);
        Logger::AddCustomOutput(&teamCityOutput);
    }

    UnitTests::TestCore::Instance()->Init(MakeFunction(this, &TestCore::OnTestClassStarted),
                                          MakeFunction(this, &TestCore::OnTestClassFinished),
                                          MakeFunction(this, &TestCore::OnTestStarted),
                                          MakeFunction(this, &TestCore::OnTestFinished),
                                          MakeFunction(this, &TestCore::OnTestFailed),
                                          MakeFunction(this, &TestCore::OnTestClassDisabled));
    if (!TArcTestCoreDetail::runOnlyTheseTestClasses.empty())
    {
        UnitTests::TestCore::Instance()->RunOnlyTheseTestClasses(TArcTestCoreDetail::runOnlyTheseTestClasses);
    }
    if (!TArcTestCoreDetail::disableTheseTestClasses.empty())
    {
        UnitTests::TestCore::Instance()->DisableTheseTestClasses(TArcTestCoreDetail::disableTheseTestClasses);
    }

    if (!UnitTests::TestCore::Instance()->HasTestClasses())
    {
        Logger::Error("%s", "There are no test classes");
        e.QuitAsync(0);
    }
    else
    {
#if defined(TEST_COVERAGE)
        RefPtr<File> covergeFile(DAVA::File::Create(TArcTestCoreDetail::TestCoverageFileName, File::CREATE | File::WRITE));
        TEST_VERIFY(covergeFile);
        covergeFile->Flush();
#endif // __DAVAENGINE_MACOS__
    }

    CreateRenderWidget();
}

void TestCore::OnAppFinished()
{
    if (TArcTestCoreDetail::teamcityOutputEnabled)
    {
        DAVA::Logger::RemoveCustomOutput(&teamCityOutput);
    }
}

void TestCore::Update(float32 delta)
{
    if (!UnitTests::TestCore::Instance()->ProcessTests(delta))
    {
        ProcessTestCoverage();
        FinishTests();
    }
}

void TestCore::ProcessTestCoverage()
{
#if defined(TEST_COVERAGE)
    // Output test coverage for sample
    Map<String, DAVA::UnitTests::TestCoverageInfo> map = UnitTests::TestCore::Instance()->GetTestCoverage();
    Logger::Info("Test coverage");

    for (const auto& x : map)
    {
        Logger::Info("  %s:", x.first.c_str());
        const Vector<String>& v = x.second.testFiles;
        for (const String& s : v)
        {
            Logger::Info("        %s", s.c_str());
        }
    }

    RefPtr<File> coverageFile(File::Create(TArcTestCoreDetail::TestCoverageFileName, File::APPEND | File::WRITE));
    DVASSERT(coverageFile);

    auto toJson = [&coverageFile](DAVA::String item) { coverageFile->Write(item.c_str(), item.size()); };

    toJson("{ \n");
    
#if defined(DAVA_UNITY_FOLDER)
    toJson("    \"UnityFolder\": \"" + DAVA::String(DAVA_UNITY_FOLDER) + "\",\n");
#endif

    toJson("    \"Coverage\":  {\n");

    for (const auto& x : map)
    {
        toJson("         \"" + x.first + "\": \"");

        const Vector<String>& v = x.second.testFiles;
        for (const String& s : v)
        {
            toJson(s + (&s != &*v.rbegin() ? " " : ""));
        }

        toJson(x.first != map.rbegin()->first ? "\",\n" : "\"\n");
    }

    toJson("     },\n");

    toJson("    \"CoverageFolders\":  {\n");

    for (const auto& x : map)
    {
        const Vector<String>& v = x.second.testFiles;
        for (const String& s : v)
        {
            toJson("         \"" + s + "\": \"");

            auto mapTargetFolders = x.second.targetFolders;
            auto it = mapTargetFolders.find(s);
            String strPast;
            if (it != mapTargetFolders.end())
            {
                strPast = it->second;
            }
            else
            {
                strPast = mapTargetFolders.find("all")->second;
            }
            toJson(strPast);
            toJson(x.first != map.rbegin()->first || s != *v.rbegin() ? "\",\n" : "\"\n");
        }
    }

    toJson("     }\n");

    toJson("}\n");
    
#endif // TEST_COVERAGE
}

void TestCore::ProcessCommandLine()
{
    CommandLineParser* cmdline = CommandLineParser::Instance();
    if (cmdline->CommandIsFound("-only_test"))
    {
        TArcTestCoreDetail::runOnlyTheseTestClasses = cmdline->GetCommandParam("-only_test");
    }
    if (cmdline->CommandIsFound("-disable_test"))
    {
        TArcTestCoreDetail::disableTheseTestClasses = cmdline->GetCommandParam("-disable_test");
    }
    if (cmdline->CommandIsFound("-noteamcity"))
    {
        TArcTestCoreDetail::teamcityOutputEnabled = false;
    }
    if (cmdline->CommandIsFound("-teamcity_capture_stdout"))
    {
        TArcTestCoreDetail::teamcityCaptureStdout = true;
    }
}

void TestCore::OnError()
{
    DVASSERT_HALT();
}

void TestCore::OnTestClassStarted(const DAVA::String& testClassName)
{
    currentTestClass = testClassName;
    Logger::Info("%s", TeamcityTestsOutput::FormatTestClassStarted(testClassName).c_str());
}

void TestCore::OnTestClassFinished(const DAVA::String& testClassName)
{
    currentTestClass.clear();
    Logger::Info("%s", TeamcityTestsOutput::FormatTestClassFinished(testClassName).c_str());
}

void TestCore::OnTestClassDisabled(const DAVA::String& testClassName)
{
    Logger::Info("%s", TeamcityTestsOutput::FormatTestClassDisabled(testClassName).c_str());
}

void TestCore::OnTestStarted(const DAVA::String& testClassName, const DAVA::String& testName)
{
    DVASSERT(currentTestClass == testClassName);
    currentTestCase = testName;
    Logger::Info("%s", TeamcityTestsOutput::FormatTestStarted(testClassName, testName).c_str());
}

void TestCore::OnTestFinished(const DAVA::String& testClassName, const DAVA::String& testName)
{
    DVASSERT(currentTestClass == testClassName);
    currentTestCase.clear();
    Logger::Info("%s", TeamcityTestsOutput::FormatTestFinished(testClassName, testName).c_str());
}

void TestCore::OnTestFailed(const String& testClassName, const String& testName, const String& condition, const char* filename, int lineno, const String& userMessage)
{
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
    OnError();
}

void TestCore::OnTestPartResult(const ::testing::TestPartResult& testPartResult)
{
    if (testPartResult.type() != ::testing::TestPartResult::kSuccess)
    {
        OnTestFailed(currentTestClass, currentTestCase, testPartResult.summary(), testPartResult.file_name(), testPartResult.line_number(), "");
    }
}

void TestCore::FinishTests()
{
    // Inform teamcity script we just finished all tests
    Logger::Debug("Finish all tests.");
    PlatformApi::Qt::GetApplication()->quit();
}

void TestCore::CreateRenderWidget()
{
    RenderWidget* w = PlatformApi::Qt::GetRenderWidget();
    w->show();
}

} // namespace TArc
} // namespace DAVA
