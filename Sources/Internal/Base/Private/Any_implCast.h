#pragma once
#ifndef __Dava_Any__
#include "Base/Any.h"
#endif

namespace DAVA
{
template <typename T>
bool AnyCast<T>::CanCast(const Any& any)
{
    return false;
}

template <typename T>
T AnyCast<T>::Cast(const Any& any)
{
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

        if (TypeInheritance::Cast(any.GetType(), inPtr, Type::Instance<P>(), &outPtr))
        {
            return static_cast<T*>(outPtr);
        }

        DAVA_THROW(Exception, "Any:: can't be casted into specified T*");
    }
};

} // namespace DAVA
