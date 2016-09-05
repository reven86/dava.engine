#include "Testing/TArcTestClass.h"
#include "TArcCore/ControllerModule.h"

#include "Engine/Public/Engine.h"
#include "Engine/Public/Qt/NativeServiceQt.h"
#include "UnitTests/UnitTests.h"

#include <QTimer>
#include <gmock/gmock-spec-builders.h>

namespace DAVA
{
namespace TArc
{

const double TestClass::testTimeLimit = 10.0; // seconds

namespace TArcTestClassDetail
{
class TestControllerModule: public ControllerModule
{
protected:
    void OnRenderSystemInitialized(Window& w) override
    {
    }

    bool CanWindowBeClosedSilently(const WindowKey& key) override
    {
        return true;
    }
    
    void SaveOnWindowClose(const WindowKey& key) override
    {
    }

    void RestoreOnWindowClose(const WindowKey& key) override
    {
    }

    void OnContextCreated(DataContext& context) override
    {
    }

    void OnContextDeleted(DataContext& context) override
    {
    }

    void PostInit() override
    {
        ContextManager& ctxManager = GetContextManager();
        DataContext::ContextID id = ctxManager.CreateContext();
        ctxManager.ActivateContext(id);
    }
};
}

TestClass::~TestClass()
{
    DVASSERT(core != nullptr);
    Engine* e = Engine::Instance();
    RenderWidget* widget = e->GetNativeService()->GetRenderWidget();
    DVASSERT(widget != nullptr);
    widget->setParent(nullptr); // remove it from Qt hierarchy to avoid Widget deletion.

    Core* c = core.release();
    QTimer::singleShot(0, [c]()
    {
        c->OnLoopStopped();
        delete c;
    });
}

void TestClass::SetUp(const String& testName)
{
    if (core == nullptr)
    {
        Engine* e = Engine::Instance();
        DVASSERT(e != nullptr);
        DVASSERT(e->IsConsoleMode() == false);
        core.reset(new Core(*e, false));

        CreateTestedModules();
        if (!core->HasControllerModule())
        {
            core->CreateModule<TArcTestClassDetail::TestControllerModule>();
        }

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
        TEST_VERIFY(::testing::Mock::VerifyAndClear());
        return true;
    }

    bool hasNotSatisfied = ::testing::Mock::HasNotSatisfiedExpectation();
    if (hasNotSatisfied == false)
    {
        TEST_VERIFY(::testing::Mock::VerifyAndClear());
    }
    return !hasNotSatisfied;
}

OperationInvoker* TestClass::GetMockInvoker()
{
    return core->GetMockInvoker();
}

DataContext& TestClass::GetActiveContext()
{
    return core->GetActiveContext();
}
    
DataContext& TestClass::GetGlobalContext()
{
    return core->GetGlobalContext();
}

DataWrapper TestClass::CreateWrapper(const DAVA::ReflectedType* type)
{
    return core->CreateWrapper(type);
}

} // namespace TArc
} // namespace DAVA