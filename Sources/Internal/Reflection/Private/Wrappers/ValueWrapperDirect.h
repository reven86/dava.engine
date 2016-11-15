#pragma once

#ifndef __DAVA_Reflection__
#include "Reflection/Reflection.h"
#endif

namespace DAVA
{
class ValueWrapperDirect : public ValueWrapper
{
public:
    ValueWrapperDirect(const RtType* type_, bool isConst_ = false)
        : type(type_)
        , isConst(isConst_ || type->IsConst())
    {
    }

    bool IsReadonly(const ReflectedObject& object) const override
    {
        return isConst || object.IsConst();
    }

    const RtType* GetType() const override
    {
        return type;
    }

    Any GetValue(const ReflectedObject& object) const override
    {
        Any ret;

        if (object.IsValid())
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
            const RtType* inType = object.GetReflectedType()->GetRtType();
            ret = value.StoreValue(ptr, inType->GetSize());
        }

        return ret;
    }

    ReflectedObject GetValueObject(const ReflectedObject& object) const override
    {
        return object;
    }

protected:
    const RtType* type;
    bool isConst;
};

} // namespace DAVA
