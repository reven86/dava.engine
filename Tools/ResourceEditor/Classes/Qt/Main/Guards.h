#ifndef __GUARDS_H__
#define __GUARDS_H__

#include "Debug/DVAssert.h"
#include <QObject>

namespace Guard
{
class ScopedBoolGuard final
{
public:
    ScopedBoolGuard(bool& value, bool newValue)
        : guardedValue(value)
        , oldValue(value)
    {
        guardedValue = newValue;
    }
    ~ScopedBoolGuard()
    {
        guardedValue = oldValue;
    };

private:
    bool& guardedValue;
    const bool oldValue;
};
}


#endif // __GUARDS_H__
