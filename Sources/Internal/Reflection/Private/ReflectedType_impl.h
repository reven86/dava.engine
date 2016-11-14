#ifndef __DAVA_ReflectedType__
#include "Reflection/ReflectedType.h"
#endif

namespace DAVA
{
namespace ReflectedTypeDetail
{
static std::array<void*, 4> zeroArray = { 0 };
};

inline const RttiType* ReflectedType::GetRttiType() const
{
    return rttiType;
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
bool ReflectedType::HasCtor(CtorWrapper::Policy policy) const
{
    if ((0 == sizeof...(Args)) && policy == CtorWrapper::Policy::ByValue && rttiType->IsFundamental())
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
Any ReflectedType::Create(CtorWrapper::Policy policy, Args... args) const
{
    if ((0 == sizeof...(Args)) && policy == CtorWrapper::Policy::ByValue && rttiType->IsFundamental())
    {
        Any ret;
        ret.LoadValue(ReflectedTypeDetail::zeroArray.data(), rttiType);
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

    // TODO:
    // decide to throw or not to throw exception
    // ...
    // Exception("There is no appropriate ctor.");

    return Any();
}

} // namespace DAVA
