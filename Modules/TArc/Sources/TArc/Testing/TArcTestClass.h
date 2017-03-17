#pragma once

#include "TArc/Core/Core.h"
#include "TArc/Testing/MockInvoker.h"
#include "TArc/Utils/QtConnections.h"

#include <QtTools/Utils/QtDelayedExecutor.h>

#include <UnitTests/TestClass.h>
#include <UnitTests/UnitTests.h>

#include <QList>
#include <QWidget>

namespace DAVA
{
namespace TArc
{
class TestClass : public UnitTests::TestClass
{
    static const double testTimeLimit;

public:
    ~TestClass();

    void Init();
    void DirectUpdate(float32 timeElapsed, const String& testName);
    bool DirectTestComplete(const String& testName) const;

    virtual void WriteInitialSettings()
    {
    }
    virtual void CreateTestedModules();

    MockInvoker* GetMockInvoker();
    DataContext* GetActiveContext();
    const DataContext* GetActiveContext() const;
    DataContext* GetGlobalContext();
    const DataContext* GetGlobalContext() const;
    DataWrapper CreateWrapper(const DAVA::ReflectedType* type);
    ContextAccessor* GetAccessor();
    const ContextAccessor* GetAccessor() const;
    ContextManager* GetContextManager();
    const ContextManager* GetContextManager() const;

    PropertiesItem CreatePropertiesItem(const String& name) const;

    template <typename... Args>
    void InvokeOperation(int operationId, const Args&... args);

    QWidget* GetWindow(const WindowKey& wndKey) const;
    QList<QWidget*> LookupWidget(const WindowKey& wndKey, const QString& objectName) const;

    template <typename T>
    T* LookupSingleWidget(const WindowKey& wndKey, const QString& objectName) const
    {
        QList<QWidget*> w = LookupWidget(wndKey, objectName);
        TEST_VERIFY(w.size() == 1);
        T* result = qobject_cast<T*>(w.front());
        TEST_VERIFY(result != nullptr);
        return result;
    }

    static Signal<Core*> coreChanged;

protected:
    virtual void AfterWrappersSync()
    {
    }

    std::unique_ptr<Core> core;
    std::unique_ptr<MockInvoker> mockInvoker;
    QtConnections connections;
    bool updateForCurrentTestCalled = false;
    DAVA::FilePath documentsPath;
};

template <typename... Args>
inline void TestClass::InvokeOperation(int operationId, const Args&... args)
{
    core->GetCoreInterface()->Invoke(operationId, args...);
}

class TestClassHolder final : public UnitTests::TestClass
{
public:
    TestClassHolder(std::unique_ptr<DAVA::TArc::TestClass>&& testClass);

    void InitTimeStampForTest(const String& testName) override;
    void SetUp(const String& testName) override;
    void TearDown(const String& testName) override;
    void Update(float32 timeElapsed, const String& testName) override;
    bool TestComplete(const String& testName) const override;
    UnitTests::TestCoverageInfo FilesCoveredByTests() const override;

    const String& TestName(size_t index) const override;
    size_t TestCount() const override;
    void RunTest(size_t index) override;

private:
    void AddCall(const Function<void()>& call) const;
    void AddCallImpl(const Function<void()>& call);
    void ProcessCalls() const;
    void ProcessCallsImpl();

private:
    std::unique_ptr<DAVA::TArc::TestClass> testClass;
    bool pendingEventProcess = false;
    Vector<Function<void()>> callsQueue;
    QtDelayedExecutor executor;
    bool currentTestFinished = false;
};

template <typename T>
class TestClassHolderFactory : public DAVA::UnitTests::TestClassFactoryBase
{
public:
    DAVA::UnitTests::TestClass* CreateTestClass()
    {
        return new DAVA::TArc::TestClassHolder(std::make_unique<T>());
    }
};

} // namespace TArc
} // namespace DAVA
