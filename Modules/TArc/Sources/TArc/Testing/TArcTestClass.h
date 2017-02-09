#pragma once

#include "TArc/Core/Core.h"
#include "TArc/Testing/MockInvoker.h"
#include "TArc/Utils/QtConnections.h"

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

    void SetUp(const String& testName) override;
    void Update(float32 timeElapsed, const String& testName) override;
    bool TestComplete(const String& testName) const override;

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
};

template <typename... Args>
inline void TestClass::InvokeOperation(int operationId, const Args&... args)
{
    core->GetCoreInterface()->Invoke(operationId, args...);
}

} // namespace TArc
} // namespace DAVA
