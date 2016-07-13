#pragma once

#include "Reflection/ReflectionWrappers.h"

namespace DAVA
{
class ValueWrapperDirect : public ValueWrapper
{
public:
    ValueWrapperDirect(const Type* type_)
        : type(type_)
    {
    }

    bool IsReadonly() const override
    {
        return type->IsConst();
    }

    const Type* GetType() const override
    {
        return type;
    }

    Any GetValue(const ReflectedObject& object) const override
    {
        Any ret;

        // TODO:
        // optimize
        // ...

        if (object.IsValid())
        {
            void* ptr = object.GetVoidPtr();
            ret.LoadValue(type, ptr);
        }

        return ret;
    }

    bool SetValue(const ReflectedObject& object, const Any& value) const override
    {
        bool ret = false;

        // TODO:
        // optimize
        // ...

        if (!IsReadonly() && object.IsValid())
        {
            void* ptr = object.GetVoidPtr();
            const Type* inType = object.GetType()->GetIndirectionType();
            value.SaveValue(ptr, inType->GetSize());

            ret = true;
        }

        return ret;
    }

    ReflectedObject GetValueObject(const ReflectedObject& object) const override
    {
        return object;
    }

protected:
    const Type* type;
};

} // namespace DAVA
