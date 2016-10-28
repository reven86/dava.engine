#pragma once
#include "Reflection/Wrappers.h"

namespace DAVA
{
class ValueWrapperDirect : public FieldWrapper
{
public:
    ValueWrapperDirect(const RttiType* type_)
        : type(type_)
    {
    }

    bool IsReadonly() const override
    {
        return type->IsConst();
    }

    const RttiType* GetType() const override
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
            ret.LoadValue(ptr, type);
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
            const RttiType* inType = object.GetType()->Deref();
            ret = value.StoreValue(ptr, inType->GetSize());
        }

        return ret;
    }

    ReflectedObject GetFieldObject(const ReflectedObject& object) const override
    {
        return object;
    }

protected:
    const RttiType* type;
};

} // namespace DAVA
