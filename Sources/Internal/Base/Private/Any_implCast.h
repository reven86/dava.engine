#pragma once

#ifndef __Dava_Any__
#include "Base/Any.h"
#endif

#include "Base/UnordererMap.h"

namespace DAVA
{
namespace AnyDetail
{
template <typename T>
struct AnyCastHolder
{
    using CastFn = T (*)(const Any&);

    static UnorderedMap<const Type*, CastFn> castFns;

    static CastFn GetCastFn(const Type* fromType)
    {
        auto it = castFns.find(fromType);
        if (it != castFns.end())
        {
            return (*it).second;
        }

        return nullptr;
    }
};

template <typename T>
UnorderedMap<const Type*, typename AnyCastHolder<T>::CastFn> AnyCastHolder<T>::castFns;

template <typename T>
struct AnyCastImpl
{
    static bool CanCast(const Any& any)
    {
        return (nullptr != AnyCastHolder<T>::GetCastFn(any.GetType()));
    }

    static T Cast(const Any& any)
    {
        auto fn = AnyCastHolder<T>::GetCastFn(any.GetType());

        if (nullptr != fn)
        {
            return (*fn)(any);
        }

        DAVA_THROW(Exception, "Any:: can't be casted into specified T");
    }

    template <typename U>
    static T Cast(const Any& any, const U& def)
    {
        auto fn = AnyCastHolder<T>::GetCastFn(any.GetType());

        if (nullptr != fn)
        {
            return (*fn)(any);
        }

        return static_cast<T>(def);
    }
};

template <typename T>
struct AnyCastImpl<T*>
{
    static bool CanCast(const Any& any)
    {
        using P = Type::DecayT<T*>;
        return TypeInheritance::CanCast(any.GetType(), Type::Instance<P>());
    }

    static T* Cast(const Any& any)
    {
        using P = Type::DecayT<T*>;

        void* inPtr = any.Get<void*>();
        void* outPtr = nullptr;

        if (TypeInheritance::Cast(any.GetType(), Type::Instance<P>(), inPtr, &outPtr))
        {
            return static_cast<T*>(outPtr);
        }

        DAVA_THROW(Exception, "Any:: can't be casted into specified T*");
    }

    template <typename U>
    static T* Cast(const Any& any, const U& def)
    {
        using P = Type::DecayT<T*>;

        void* inPtr = any.Get<void*>();
        void* outPtr = nullptr;

        if (TypeInheritance::Cast(any.GetType(), Type::Instance<P>(), inPtr, &outPtr))
        {
            return static_cast<T*>(outPtr);
        }

        return static_cast<T*>(def);
    }
};
} // AnyDetail

template <typename From, typename To>
void AnyCast<From, To>::Register(To (*fn)(const Any&))
{
    AnyDetail::AnyCastHolder<To>::castFns[Type::Instance<From>()] = fn;
}

template <typename From, typename To>
void AnyCast<From, To>::RegisterDefault()
{
    AnyDetail::AnyCastHolder<To>::castFns[Type::Instance<From>()] = [](const Any& any) {
        return static_cast<To>(any.Get<From>());
    };
}

} // namespace DAVA
