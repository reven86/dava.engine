#include "TArc/Testing/TArcTestClass.h"
#include "TArc/Testing/MockInvoker.h"
#include "TArc/Core/ControllerModule.h"
#include "TArc/WindowSubSystem/UI.h"
#include "TArc/Utils/DebuggerDetection.h"

#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <FileSystem/FileSystem.h>
#include <UnitTests/UnitTests.h>

#include <QTimer>
#include <QApplication>
#include <QAbstractEventDispatcher>
#include <gmock/gmock-spec-builders.h>

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
        DataContext::ContextID id = ctxManager->CreateContext(DAVA::Vector<std::unique_ptr<DataNode>>());
        ctxManager->ActivateContext(id);
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(TestControllerModule, ControllerModule)
    {
        ReflectionRegistrator<TestControllerModule>::Begin()
        .ConstructorByPointer()
        .End();
    }
};
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

    FilePath prevDocPath = documentsPath;
    coreChanged.Emit(nullptr);
    Core* c = core.release();
    c->syncSignal.DisconnectAll();
    c->SetInvokeListener(nullptr);
    mockInvoker.reset();
    QTimer::singleShot(0, [c, prevDocPath]()
                       {
                           c->OnLoopStopped();
                           delete c;

                           const EngineContext* ctx = GetEngineContext();
                           FilePath tmpDirectory = ctx->fileSystem->GetCurrentDocumentsDirectory();
                           if (ctx->fileSystem->Exists(tmpDirectory))
                           {
                               ctx->fileSystem->DeleteDirectory(tmpDirectory, true);
                           }
                           ctx->fileSystem->SetCurrentDocumentsDirectory(prevDocPath);
                       });
}

void TestClass::Init()
{
    updateForCurrentTestCalled = false;
    if (core == nullptr)
    {
        const EngineContext* ctx = GetEngineContext();
        documentsPath = ctx->fileSystem->GetCurrentDocumentsDirectory();
        FilePath tmpDirectory = ctx->fileSystem->GetTempDirectoryPath() + "/SelfTestFolder/";
        if (ctx->fileSystem->Exists(tmpDirectory))
        {
            ctx->fileSystem->DeleteDirectory(tmpDirectory, true);
        }

        ctx->fileSystem->CreateDirectory(tmpDirectory, true);
        ctx->fileSystem->SetCurrentDocumentsDirectory(tmpDirectory);

        WriteInitialSettings();

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
        core->syncSignal.Connect(this, &TestClass::AfterWrappersSync);
        coreChanged.Emit(core.get());
    }
}

void TestClass::DirectUpdate(float32 timeElapsed, const String& testName)
{
    DVASSERT(core != nullptr);
    core->OnFrame(timeElapsed);
    updateForCurrentTestCalled = true;
}

bool TestClass::DirectTestComplete(const String& testName) const
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
    checkTimeLimit = !IsDebuggerPresent();
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
    return !hasNotSatisfied && updateForCurrentTestCalled;
}

MockInvoker* TestClass::GetMockInvoker()
{
    return mockInvoker.get();
}

DataContext* TestClass::GetActiveContext()
{
    return core->GetCoreInterface()->GetActiveContext();
}

const DataContext* TestClass::GetActiveContext() const
{
    const Core* corePtr = core.get();
    return corePtr->GetCoreInterface()->GetActiveContext();
}

DataContext* TestClass::GetGlobalContext()
{
    return core->GetCoreInterface()->GetGlobalContext();
}

const DataContext* TestClass::GetGlobalContext() const
{
    const Core* corePtr = core.get();
    return corePtr->GetCoreInterface()->GetGlobalContext();
}

DataWrapper TestClass::CreateWrapper(const DAVA::ReflectedType* type)
{
    return core->GetCoreInterface()->CreateWrapper(type);
}

ContextAccessor* TestClass::GetAccessor()
{
    return core->GetCoreInterface();
}

const ContextAccessor* TestClass::GetAccessor() const
{
    const Core* corePtr = core.get();
    return corePtr->GetCoreInterface();
}

ContextManager* TestClass::GetContextManager()
{
    return core->GetCoreInterface();
}

const ContextManager* TestClass::GetContextManager() const
{
    const Core* corePtr = core.get();
    return corePtr->GetCoreInterface();
}

PropertiesItem TestClass::CreatePropertiesItem(const String& name) const
{
    return core->GetCoreInterface()->CreatePropertiesNode(name);
}

QWidget* TestClass::GetWindow(const WindowKey& wndKey) const
{
    UI* manager = core->GetUI();
    QWidget* wnd = manager->GetWindow(wndKey);

    return wnd;
}

QList<QWidget*> TestClass::LookupWidget(const WindowKey& wndKey, const QString& objectName) const
{
    return GetWindow(wndKey)->findChildren<QWidget*>(objectName);
}

void TestClass::CreateTestedModules()
{
}

Signal<Core*> TestClass::coreChanged;

TestClassHolder::TestClassHolder(std::unique_ptr<DAVA::TArc::TestClass>&& testClass_)
    : testClass(std::move(testClass_))
{
}

void TestClassHolder::InitTimeStampForTest(const String& testName)
{
    testClass->InitTimeStampForTest(testName);
}

void TestClassHolder::SetUp(const String& testName)
{
    currentTestFinished = false;
    testClass->Init();
    AddCall([this, testName]()
            {
                testClass->SetUp(testName);
            });
}

void TestClassHolder::TearDown(const String& testName)
{
    DVASSERT(currentTestFinished == true);
    testClass->TearDown(testName);
}

void TestClassHolder::Update(float32 timeElapsed, const String& testName)
{
    if (currentTestFinished == true)
    {
        return;
    }

    testClass->DirectUpdate(timeElapsed, testName);
    AddCall([this, timeElapsed, testName]()
            {
                testClass->Update(timeElapsed, testName);
            });
}

bool TestClassHolder::TestComplete(const String& testName) const
{
    if (currentTestFinished == true)
    {
        return true;
    }

    TestClassHolder* nonConst = const_cast<TestClassHolder*>(this);
    AddCall([nonConst, testName]()
            {
                bool testCompleted = nonConst->testClass->TestComplete(testName);
                if (testCompleted == true)
                {
                    testCompleted = nonConst->testClass->DirectTestComplete(testName);
                }

                nonConst->currentTestFinished = testCompleted;
            });

    return false;
}

DAVA::UnitTests::TestCoverageInfo TestClassHolder::FilesCoveredByTests() const
{
    return testClass->FilesCoveredByTests();
}

const DAVA::String& TestClassHolder::TestName(size_t index) const
{
    return testClass->TestName(index);
}

size_t TestClassHolder::TestCount() const
{
    return testClass->TestCount();
}

void TestClassHolder::RunTest(size_t index)
{
    AddCall([this, index]()
            {
                testClass->RunTest(index);
            });
}

void TestClassHolder::AddCall(const DAVA::Function<void()>& call) const
{
    const_cast<TestClassHolder*>(this)->AddCallImpl(call);
}

void TestClassHolder::AddCallImpl(const Function<void()>& call)
{
    callsQueue.push_back(call);
    if (pendingEventProcess == false)
    {
        pendingEventProcess = true;
        executor.DelayedExecute(MakeFunction(this, &TestClassHolder::ProcessCallsImpl));
    }
}

void TestClassHolder::ProcessCalls() const
{
    const_cast<TestClassHolder*>(this)->ProcessCallsImpl();
}

void TestClassHolder::ProcessCallsImpl()
{
    DVASSERT(pendingEventProcess == true);
    Vector<DAVA::Function<void()>> queue = callsQueue;
    callsQueue.clear();
    pendingEventProcess = false;

    for (const DAVA::Function<void()>& fn : queue)
    {
        fn();
        if (currentTestFinished == true)
        {
            callsQueue.clear();
            break;
        }
    }
}

} // namespace TArc
} // namespace DAVA