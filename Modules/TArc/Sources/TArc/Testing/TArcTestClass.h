#pragma once

#include "TArc/Core/Core.h"
#include "TArc/Testing/MockInvoker.h"
#include "TArc/Utils/QtConnections.h"

#include "UnitTests/TestClass.h"

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
    DataContext* GetGlobalContext();
    DataWrapper CreateWrapper(const DAVA::ReflectedType* type);
    ContextAccessor* GetAccessor();
    ContextManager* GetContextManager();

    QWidget* GetWindow(const WindowKey& wndKey) const;
    QList<QWidget*> LookupWidget(const WindowKey& wndKey, const QString& objectName) const;

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

} // namespace TArc
} // namespace DAVA
