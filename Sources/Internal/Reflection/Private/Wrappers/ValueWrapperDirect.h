#pragma once

#ifndef __DAVA_Reflection__
#include "Reflection/Reflection.h"
#endif

namespace DAVA
{
class ValueWrapperDirect : public ValueWrapper
{
public:
    ValueWrapperDirect(const Type* type_, bool isConst_ = false)
        : type(type_)
        , isConst(isConst_ || type->IsConst())
    {
    }

    bool IsReadonly(const ReflectedObject& object) const override
    {
        return isConst || object.IsConst();
    }

    const Type* GetType() const override
    {
        return type;
    }

    Any GetValue(const ReflectedObject& object) const override
    {
        Any ret;

        if (!IsReadonly(object))
        {
            void* ptr = object.GetVoidPtr();
            ret.LoadValue(ptr, type);
        }

        return ret;
    }

    bool SetValue(const ReflectedObject& object, const Any& value) const override
    {
        bool ret = false;

        if (!IsReadonly(object) && object.IsValid())
        {
            void* ptr = object.GetVoidPtr();
            const Type* inType = object.GetReflectedType()->GetType();
            ret = value.StoreValue(ptr, inType->GetSize());
        }

        return ret;
    }

    ReflectedObject GetValueObject(const ReflectedObject& object) const override
    {
        return object;
    }

protected:
    const Type* type;
    bool isConst;
};

} // namespace DAVA
