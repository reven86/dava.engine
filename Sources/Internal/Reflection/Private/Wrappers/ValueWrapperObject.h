#pragma once

#ifndef __DAVA_Reflection__
#include "Reflection/Reflection.h"
#endif

namespace DAVA
{
class ValueWrapperObject : public ValueWrapper
{
public:
    ValueWrapperObject()
    {
    }

    bool IsReadonly(const ReflectedObject& object) const override
    {
        return object.IsConst();
    }

    const Type* GetType(const ReflectedObject& object) const override
    {
        if (object.IsConst())
        {
            return Type::Instance<const void*>();
        }
        else
        {
            return Type::Instance<void*>();
        }
    }

    Any GetValue(const ReflectedObject& object) const override
    {
        return Any(object.GetVoidPtr());
    }

    bool SetValue(const ReflectedObject& object, const Any& value) const override
    {
        return false;
    }

    bool SetValueWithCast(const ReflectedObject& object, const Any& value) const override
    {
        return false;
    }

    ReflectedObject GetValueObject(const ReflectedObject& object) const override
    {
        return object;
    }
};

} // namespace DAVA
