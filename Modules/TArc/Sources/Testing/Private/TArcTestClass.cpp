#include "Testing/TArcTestClass.h"

#include "UnitTests/UnitTests.h"
#include "Engine/Public/Engine.h"

#include <gmock/gmock-spec-builders.h>

namespace DAVA
{
namespace TArc
{

const double TestClass::testTimeLimit = 30.0; // seconds

TestClass::~TestClass()
{
    DVASSERT(core != nullptr);
    Engine* e = Engine::Instance();
    RenderWidget* widget = e->GetNativeService()->GetRenderWidget();
    DVASSERT(widget != nullptr);
    widget->setParent(nullptr); // remove it from Qt hierarchy to avoid Widget deletion.
    core->OnLoopStopped();
}

void TestClass::SetUp(const String& testName)
{
    if (core == nullptr)
    {
        Engine* e = Engine::Instance();
        DVASSERT(e != nullptr);
        DVASSERT(e->IsConsoleMode() == false);
        core.reset(new Core(*e));

        CreateTestedModules();

        core->OnLoopStarted();
        Window* w = e->PrimaryWindow();
        DVASSERT(w);
        core->OnWindowCreated(*w);
    }

    DAVA::UnitTests::TestClass::SetUp(testName);
}

void TestClass::Update(float32 timeElapsed, const String& testName)
{
    DVASSERT(core != nullptr);
    core->OnFrame(timeElapsed);
}

bool TestClass::TestComplete(const String& testName) const
{
    DVASSERT(core != nullptr);
    auto iter = std::find_if(tests.begin(), tests.end(), [&testName](const TestInfo& testInfo)
    {
        return testInfo.name == testName;
    });

    DVASSERT(iter != tests.end());
    using namespace std::chrono;
    double elapsedSeconds = duration_cast<duration<double>>(TestInfo::Clock::now() - iter->startTime).count();
    if (elapsedSeconds > testTimeLimit)
    {
        TEST_VERIFY(::testing::Mock::VerifyAndClear(nullptr));
        return true;
    }

    return ::testing::Mock::HasNotSatisfiedExpectation() == false;
}

} // namespace TArc
} // namespace DAVA