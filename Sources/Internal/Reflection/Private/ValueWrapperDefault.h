#pragma once
#include "Reflection/Wrappers.h"

namespace DAVA
{
template <typename T>
class ValueWrapperDefault : public ValueWrapper
{
    static const bool is_const = std::is_const<T>::value;

public:
    ValueWrapperDefault() = default;

    bool IsReadonly() const override
    {
        return is_const;
    }

    const RttiType* GetType() const override
    {
        return RttiType::Instance<T>();
    }

    Any GetValue(const ReflectedObject& object) const override
    {
        Any ret;

        const T* ptr = object.GetPtr<const T>();
        ret.Set(*ptr);

        return ret;
    }

    bool SetValue(const ReflectedObject& object, const Any& value) const override
    {
        T* ptr = object.GetPtr<T>();
        return SetValueInternal(ptr, value);
    }

    ReflectedObject GetValueObject(const ReflectedObject& object) const override
    {
        return object;
    }

protected:
    inline bool SetValueInternal(T* ptr, const Any& value) const
    {
        return SetValueInternalImpl<is_const>::fn(ptr, value);
    }

private:
    template <bool is_const, typename U = void>
    struct SetValueInternalImpl
    {
        inline static bool fn(T* ptr, const Any& value)
        {
            return false;
        }
    };

    template <typename U>
    struct SetValueInternalImpl<false, U>
    {
        inline static bool fn(T* ptr, const Any& value)
        {
            *ptr = value.Get<T>();
            return true;
        }
    };
};

} // namespace DAVA
