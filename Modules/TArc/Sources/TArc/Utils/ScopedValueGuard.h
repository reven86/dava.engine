#pragma once

#include "Debug/DVAssert.h"
#include <QObject>

#define SCOPED_VALUE_GUARD(type, var, value, retValue) \
    if (var == value) \
    { \
        return retValue; \
    } \
    DAVA::TArc::ScopedValueGuard<type> guard(var, value);

namespace DAVA
{
namespace TArc
{
template <typename T>
class ScopedValueGuard final
{
public:
    ScopedValueGuard(T& value, T newValue)
        : guardedValue(value)
        , oldValue(value)
    {
        guardedValue = newValue;
    }
    ~ScopedValueGuard()
    {
        guardedValue = oldValue;
    };

private:
    T& guardedValue;
    const T oldValue;
};
}
}
