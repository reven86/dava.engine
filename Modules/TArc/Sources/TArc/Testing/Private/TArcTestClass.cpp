#include "TArc/Testing/TArcTestClass.h"
#include "TArc/Testing/MockInvoker.h"
#include "TArc/Core/ControllerModule.h"
#include "TArc/WindowSubSystem/UI.h"

#include "Engine/Engine.h"
#include "UnitTests/UnitTests.h"

#include "Base/Platform.h"

#include <QTimer>
#include <QApplication>
#include <QAbstractEventDispatcher>
#include <gmock/gmock-spec-builders.h>


#if defined(__DAVAENGINE_MACOS__)
#include <sys/types.h>
#include <sys/sysctl.h>

#endif

namespace DAVA
{
namespace TArc
{
namespace TArcTestClassDetail
{
class TestControllerModule : public ControllerModule
{
protected:
    void OnRenderSystemInitialized(Window* w) override
    {
    }

    bool CanWindowBeClosedSilently(const WindowKey& key, String& requestWindowText) override
    {
        return true;
    }

    void SaveOnWindowClose(const WindowKey& key) override
    {
    }

    void RestoreOnWindowClose(const WindowKey& key) override
    {
    }

    void OnContextCreated(DataContext* context) override
    {
    }

    void OnContextDeleted(DataContext* context) override
    {
    }

    void PostInit() override
    {
        ContextManager* ctxManager = GetContextManager();
        DataContext::ContextID id = ctxManager->CreateContext(DAVA::Vector<std::unique_ptr<DAVA::TArc::DataNode>>());
        ctxManager->ActivateContext(id);
    }

    DAVA_VIRTUAL_REFLECTION(TestControllerModule, ControllerModule)
    {
        ReflectionRegistrator<TestControllerModule>::Begin()
        .ConstructorByPointer()
        .End();
    }
};

bool IsDebuggerPresent()
{
#if defined(__DAVAENGINE_WIN32__)
    return ::IsDebuggerPresent();
#elif defined(__DAVAENGINE_MACOS__)
    int mib[4];
    struct kinfo_proc info;
    size_t size = sizeof(info);

    info.kp_proc.p_flag = 0;

    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PID;
    mib[3] = getpid();

    sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, NULL, 0);

    return (info.kp_proc.p_flag & P_TRACED) != 0;
#else
    return false;
#endif
}
}

const double TestClass::testTimeLimit = 10.0; // seconds

TestClass::~TestClass()
{
    DVASSERT(core != nullptr);
    RenderWidget* widget = PlatformApi::Qt::GetRenderWidget();
    DVASSERT(widget != nullptr);
    widget->setParent(nullptr); // remove it from Qt hierarchy to avoid Widget deletion.

    QWidget* focusWidget = PlatformApi::Qt::GetApplication()->focusWidget();
    if (focusWidget != nullptr)
    {
        focusWidget->clearFocus();
    }

    coreChanged.Emit(nullptr);
    Core* c = core.release();
    c->SetInvokeListener(nullptr);
    mockInvoker.reset();
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
        using namespace std::chrono;
        TestInfo::TimePoint startTimePoint = TestInfo::Clock::now();
        auto timeoutCrashHandler = [startTimePoint]()
        {
            double elapsedSeconds = duration_cast<duration<double>>(TestInfo::Clock::now() - startTimePoint).count();
            if (elapsedSeconds > 10 * 60) // 10 minutes
            {
                TEST_VERIFY_WITH_MESSAGE(false, "Timeout fail");
                std::terminate();
            }
        };

        QAbstractEventDispatcher* dispatcher = qApp->eventDispatcher();
        connections.AddConnection(dispatcher, &QAbstractEventDispatcher::aboutToBlock, timeoutCrashHandler);
        connections.AddConnection(dispatcher, &QAbstractEventDispatcher::awake, timeoutCrashHandler);

        Engine* e = Engine::Instance();
        DVASSERT(e != nullptr);
        DVASSERT(e->IsConsoleMode() == false);
        core.reset(new Core(*e, false));

        mockInvoker.reset(new MockInvoker());
        core->SetInvokeListener(mockInvoker.get());

        CreateTestedModules();
        if (!core->HasControllerModule())
        {
            core->CreateModule<TArcTestClassDetail::TestControllerModule>();
        }

        core->OnLoopStarted();
        Window* w = e->PrimaryWindow();
        DVASSERT(w);
        core->OnWindowCreated(w);
        coreChanged.Emit(core.get());
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
    bool checkTimeLimit = true;
    checkTimeLimit = !TArcTestClassDetail::IsDebuggerPresent();
    if (checkTimeLimit == true && elapsedSeconds > testTimeLimit)
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

MockInvoker* TestClass::GetMockInvoker()
{
    return mockInvoker.get();
}

DataContext* TestClass::GetActiveContext()
{
    return core->GetCoreInterface()->GetActiveContext();
}

DataContext* TestClass::GetGlobalContext()
{
    return core->GetCoreInterface()->GetGlobalContext();
}

DataWrapper TestClass::CreateWrapper(const DAVA::ReflectedType* type)
{
    return core->GetCoreInterface()->CreateWrapper(type);
}

DAVA::TArc::ContextAccessor* TestClass::GetAccessor()
{
    return core->GetCoreInterface();
}

DAVA::TArc::ContextManager* TestClass::GetContextManager()
{
    return core->GetCoreInterface();
}

QWidget* TestClass::GetWindow(const WindowKey& wndKey)
{
    UIManager* manager = dynamic_cast<UIManager*>(core->GetUI());
    QWidget* wnd = manager->GetWindow(wndKey);

    return wnd;
}

QList<QWidget*> TestClass::LookupWidget(const WindowKey& wndKey, const QString& objectName)
{
    return GetWindow(wndKey)->findChildren<QWidget*>(objectName);
}

void TestClass::CreateTestedModules()
{
}

Signal<Core*> TestClass::coreChanged;

// ContextAccessor* TestClass::GetAccessor()
// {
//     return core->GetAccessor();
// }

} // namespace TArc
} // namespace DAVA