#pragma once
#include "Reflection/Private/ValueWrapperDefault.h"

#if !defined(__DAVAENGINE_ANDROID__)

namespace DAVA
{
template <typename T, typename C>
class ValueWrapperClass : public ValueWrapperDefault<T>
{
public:
    ValueWrapperClass(T C::*field_)
        : field(field_)
    {
    }

    inline Any GetValue(const ReflectedObject& object) const override
    {
        Any ret;

        C* cls = object.GetPtr<C>();
        ret.Set(cls->*field);

        return ret;
    }

    inline bool SetValue(const ReflectedObject& object, const Any& value) const override
    {
        C* cls = object.GetPtr<C>();
        T* ptr = &(cls->*field);

        return ValueWrapperDefault<T>::SetValueInternal(ptr, value);
    }

    inline ReflectedObject GetValueObject(const ReflectedObject& object) const override
    {
        C* cls = object.GetPtr<C>();
        T* ptr = &(cls->*field);

        return ReflectedObject(ptr);
    }

protected:
    T C::*field;
};

} // namespace DAVA

#endif