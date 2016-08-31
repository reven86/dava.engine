#pragma once

#include "UnitTests/TestClass.h"

namespace DAVA
{
namespace TArc
{

class Core;
class TestClass: UnitTests::TestClass
{
public:
    void Update(float32 timeElapsed, const String& testName) override;
    bool TestComplete(const String& testName) const override;

    void Init(Core& core);

    virtual void CreateTestedModules();

private:
    Core& core;
};

} // namespace TArc
} // namespace DAVA