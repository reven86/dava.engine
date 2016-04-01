#pragma once
#include "ValueWrapperDefault.h"

namespace DAVA
{
template <typename T>
class ValueWrapperStatic : public ValueWrapperDefault<T>
{
public:
    ValueWrapperStatic(T* field_)
        : field(field_)
    {
    }

    Any GetValue(const ReflectedObject& object) const override
    {
        Any ret;
        ret.Set(*field);
        return ret;
    }

    bool SetValue(const ReflectedObject& object, const Any& value) const override
    {
        return ValueWrapperDefault<T>::SetValueInternal(field, value);
    }

    ReflectedObject GetValueObject(const ReflectedObject& object) const override
    {
        return ReflectedObject(field);
    }

protected:
    T* field;
};

} // namespace DAVA
