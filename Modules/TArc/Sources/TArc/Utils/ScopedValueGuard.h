#pragma once

#include "Debug/DVAssert.h"
#include <QObject>

/*
This macro can be used to protect your class from recursive calls
 For example you have class with method Foo() and Bar(). Foo in some situations can produce call of Bar,
 And Bar can produce call of Foo. To avoid this kind of situations you can make bool field in your class and
 and write this macro at the very beginning of Foo and Bar
 Example:
    class MyClass
    {
    public:
        void Foo();
        bool Bar();

    private:
        bool guardedVariable = false;
    }

    void MyClass::Foo()
    {
        SCOPED_VALUE_GUARD(bool, guardedVariable, true, void());
        some logic here
        if (something_happend)
            emit Some ignal that will produce call of Bar();
    }

    bool MyClass::Bar()
    {
        SCOPED_VALUE_GUARD(bool, guardedVariable, true, false);
        some logic here
        if (something_happend)
            emit Some ignal that will produce call of Foo();
    }
*/
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
