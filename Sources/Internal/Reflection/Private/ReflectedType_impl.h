#pragma once

#ifndef __DAVA_ReflectedType__
#include "Reflection/Reflection.h"
#endif

namespace DAVA
{
namespace ReflectedTypeDetail
{
static std::array<void*, 4> zeroArray = { 0 };
};

inline const RtType* ReflectedType::GetRtType() const
{
    return rtType;
}

inline const String& ReflectedType::GetPermanentName() const
{
    return permanentName;
}

inline const ReflectedStructure* ReflectedType::GetStrucutre() const
{
    return structure.get();
}

inline const StructureWrapper* ReflectedType::GetStrucutreWrapper() const
{
    return structureWrapper.get();
}

template <typename... Args>
bool ReflectedType::HasCtor(ReflectionCtorPolicy policy) const
{
    if ((0 == sizeof...(Args)) && policy == ReflectionCtorPolicy::ByValue && rtType->IsFundamental())
    {
        return true;
    }
    else
    {
        auto params = DAVA::AnyFn::Params::FromArgs<Args...>();

        for (auto& ctor : structure->ctors)
        {
            if (ctor->GetCtorPolicy() == policy && ctor->GetInvokeParams() == params)
            {
                return true;
            }
        }
    }

    return false;
}

template <typename... Args>
Any ReflectedType::Create(ReflectionCtorPolicy policy, Args... args) const
{
    if ((0 == sizeof...(Args)) && policy == ReflectionCtorPolicy::ByValue && rtType->IsFundamental())
    {
        Any ret;
        ret.LoadValue(ReflectedTypeDetail::zeroArray.data(), rtType);
        return ret;
    }
    else
    {
        auto params = DAVA::AnyFn::Params::FromArgs<Args...>();

        for (auto& ctor : structure->ctors)
        {
            if (ctor->GetCtorPolicy() == policy && ctor->GetInvokeParams() == params)
            {
                return ctor->Create(std::forward<Args>(args)...);
            }
        }
    }

    DAVA_THROW(Exception, "There is no appropriate ctor to call it with specified Args...");
}
} // namespace DAVA
