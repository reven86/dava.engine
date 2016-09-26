#pragma once

#include "Base/BaseTypes.h"
#include "Logger/TeamCityTestsOutput.h"
#include "Logger/Logger.h"

#include <gtest/gtest.h>

namespace DAVA
{
class Engine;
namespace TArc
{
class TestCore : ::testing::EmptyTestEventListener
{
public:
    TestCore(Engine& e);
    ~TestCore();

private:
    void OnAppStarted();
    void OnAppFinished();
    void Update(float32 delta);

    void ProcessCommandLine();
    void ProcessTestCoverage();
    void OnError();
    void OnTestClassStarted(const DAVA::String& testClassName);
    void OnTestClassFinished(const DAVA::String& testClassName);
    void OnTestClassDisabled(const DAVA::String& testClassName);
    void OnTestStarted(const DAVA::String& testClassName, const DAVA::String& testName);
    void OnTestFinished(const DAVA::String& testClassName, const DAVA::String& testName);
    void OnTestFailed(const String& testClassName, const String& testName, const String& condition, const char* filename, int lineno, const String& userMessage);
    void OnTestPartResult(const ::testing::TestPartResult& test_part_result);
    void FinishTests();

    void CreateRenderWidget();

private:
    Engine& e;
    DAVA::TeamcityTestsOutput teamCityOutput;
    int argc = 0;
    Vector<char*> argv;
    String currentTestClass;
    String currentTestCase;
};

} // namespace TArc
} // namespace DAVA
