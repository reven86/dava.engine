#pragma once

#include "UnitTests/TestClassFactory.h"

namespace DAVA
{
namespace TArc
{
template<typename T>
class TestClassFactory : public DAVA::UnitTests::TestClassFactoryBase
{
    TestClass* CreateTestClass()
    {
        return new T;
    }
};
} // namespace TArc
} // namespace DAVA
