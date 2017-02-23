#pragma once

#include <Debug/DVAssert.h>

namespace DAVA
{
namespace TArc
{
template <typename T>
class TestModuleHolder
{
public:
    TestModuleHolder(T* instance)
    {
        DVASSERT(moduleInstance == nullptr);
        moduleInstance = instance;
    }

    static T* moduleInstance;
};

template <typename T>
T* TestModuleHolder<T>::moduleInstance = nullptr;

} // namespace TArc
} // namespace DAVA