#pragma once

#include "TArcCore/TArcCore.h"

#include "UnitTests/TestClass.h"

namespace DAVA
{
namespace TArc
{

class TestClass: public UnitTests::TestClass
{
    static const double testTimeLimit;
public:
    ~TestClass();

    void SetUp(const String& testName) override; 
    void Update(float32 timeElapsed, const String& testName) override;
    bool TestComplete(const String& testName) const override;

    virtual void CreateTestedModules() {}

protected:
    OperationInvoker* GetMockInvoker();
    DataContext& GetActiveContext();
    DataContext& GetGlobalContext();
    DataWrapper CreateWrapper(const DAVA::ReflectedType* type);

protected:
    std::unique_ptr<Core> core;
};

} // namespace TArc
} // namespace DAVA