#pragma once

#ifndef __DAVA_Reflection__
#include "Reflection/Reflection.h"
#endif

#include "Reflection/Private/Wrappers/ValueWrapperDefault.h"

#define DAVA_REFLECTION__IMPL(Cls) \
    template <typename FT__> \
    friend struct DAVA::ReflectedTypeDBDetail::ReflectionInitializerRunner; \
    static void __ReflectionInitializer() \
    { \
        static_assert(!std::is_base_of<DAVA::ReflectionBase, Cls>::value, "Use DAVA_VIRTUAL_REFLECTION for classes derived from ReflectionBase"); \
        __ReflectionInitializer_Impl(); \
    } \
    static void __ReflectionInitializer_Impl()

#define DAVA_VIRTUAL_REFLECTION__IMPL(Cls, ...) \
    template <typename FT__> \
    friend struct DAVA::ReflectedTypeDBDetail::ReflectionInitializerRunner; \
    const DAVA::ReflectedType* GetReflectedType() const override \
    { \
        return DAVA::ReflectedTypeDBDetail::GetByThisPointer(this); \
    } \
    static void __ReflectionInitializer() \
    { \
        static_assert(std::is_base_of<DAVA::ReflectionBase, Cls>::value, "Use DAVA_REFLECTION for classes that didn't derived from ReflectionBase"); \
        DAVA::ReflectedTypeDB::RegisterBases<Cls, ##__VA_ARGS__>(); \
        __ReflectionInitializer_Impl(); \
    } \
    static void __ReflectionInitializer_Impl()

#define DAVA_REFLECTION_IMPL__IMPL(Cls) \
    void Cls::__ReflectionInitializer_Impl()

namespace DAVA
{
inline bool Reflection::IsReadonly() const
{
    return valueWrapper->IsReadonly(object);
}

inline const Type* Reflection::GetValueType() const
{
    return valueWrapper->GetType();
}

inline ReflectedObject Reflection::GetValueObject() const
{
    return valueWrapper->GetValueObject(object);
}

inline Any Reflection::GetValue() const
{
    return valueWrapper->GetValue(object);
}

inline bool Reflection::SetValue(const Any& value) const
{
    return valueWrapper->SetValue(object, value);
}

inline bool Reflection::IsValid() const
{
    return (nullptr != valueWrapper && object.IsValid());
}

template <typename Meta>
inline bool Reflection::HasMeta() const
{
    return (nullptr != meta) ? meta->template HasMeta<Meta>() : false;
}

template <typename Meta>
inline const Meta* Reflection::GetMeta() const
{
    return (nullptr != meta) ? meta->template GetMeta<Meta>() : nullptr;
}

template <typename T>
Reflection Reflection::Create(T* objectPtr, const ReflectedMeta* objectMeta)
{
    if (nullptr != objectPtr)
    {
        static ValueWrapperDefault<T> objectValueWrapper;
        return Reflection(ReflectedObject(objectPtr), &objectValueWrapper, nullptr, objectMeta);
    }

    return Reflection();
}

template <>
struct AnyCompare<Reflection>
{
    static bool IsEqual(const Any& v1, const Any& v2)
    {
        const Reflection r1 = v1.Get<Reflection>();
        const Reflection r2 = v2.Get<Reflection>();
        return r1.GetValueObject() == r2.GetValueObject();
    }
};

template <>
struct AnyCompare<Vector<Reflection>>
{
    static bool IsEqual(const Any& v1, const Any& v2)
    {
        const Vector<Reflection> r1 = v1.Get<Vector<Reflection>>();
        const Vector<Reflection> r2 = v2.Get<Vector<Reflection>>();
        if (r1.size() != r2.size())
        {
            return false;
        }

        for (size_t i = 0; i < r1.size(); ++i)
        {
            if (AnyCompare<Reflection>::IsEqual(r1[i], r2[i]) == false)
            {
                return false;
            }
        }
        return true;
    }
};
} // namespace DAVA
