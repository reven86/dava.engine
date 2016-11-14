#pragma once
#include "Reflection/Wrappers.h"

namespace DAVA
{
class ValueWrapperDirect : public ValueWrapper
{
public:
    ValueWrapperDirect(const RttiType* type_, bool isConst_ = false)
        : type(type_)
        , isConst(isConst_ || type->IsConst())
    {
    }

    bool IsReadonly() const override
    {
        return isConst;
    }

    const RttiType* GetType() const override
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

        if (!IsReadonly() && object.IsValid())
        {
            void* ptr = object.GetVoidPtr();
            const RttiType* inType = object.GetType()->Deref();
            ret = value.StoreValue(ptr, inType->GetSize());
        }

        return ret;
    }

    ReflectedObject GetValueObject(const ReflectedObject& object) const override
    {
        return object;
    }

protected:
    const RttiType* type;
    bool isConst;
};

} // namespace DAVA
