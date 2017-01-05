#pragma once

#ifndef __Dava_Any__
#include "Base/Any.h"
#endif

#include "Base/UnordererMap.h"

namespace DAVA
{
#if 0
namespace AnyDetail
{
template<typename F>
struct AnyCastProtoImpl
{
    static UnorderedMap<const Type*, void*> castFns;

    template <typename T>
    static T(*)(const Any&) GetCastFn()
    {
        void *f = nullptr;

        return static_cast<T(*)(const Any&)>(f);
    }
};
} // AnyDetail
#endif

template <typename T>
bool AnyCast<T>::CanCast(const Any& any)
{
    if (std::is_fundamental<T>::value)
    {
        const Type* t = any.GetType();
        return (nullptr != t && t->IsFundamental());
    }

    return false;
}

template <typename T>
T AnyCast<T>::Cast(const Any& any)
{
    if (CanCast(any))
    {
        const void* data = any.GetData();
        size_t sz = any.GetType()->GetSize();
    }

    DAVA_THROW(Exception, "Any:: can't be casted into specified T");
}

template <typename T>
struct AnyCast<T*>
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
};

} // namespace DAVA
