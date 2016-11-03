#pragma once

#ifndef __Dava_RttiInheritance__
#include "Base/RttiInheritance.h"
#endif

namespace DAVA
{
inline const RttiInheritance::InheritanceMap& RttiInheritance::GetBaseTypes() const
{
    return baseTypes;
}

inline const RttiInheritance::InheritanceMap& RttiInheritance::GetDerivedTypes() const
{
    return derivedTypes;
}

template <typename T, typename... Bases>
void RttiInheritance::RegisterBases()
{
    const RttiType* type = RttiType::Instance<T>();

    bool basesUnpack[] = { false, RttiInheritance::AddBaseType<T, Bases>()... };
    bool derivedUnpack[] = { false, RttiInheritance::AddDerivedType<Bases, T>()... };
}

template <typename T, typename B>
bool RttiInheritance::AddBaseType()
{
    const RttiType* type = RttiType::Instance<T>();
    const RttiInheritance* inheritance = type->GetInheritance();

    if (nullptr == inheritance)
    {
        inheritance = new RttiInheritance();
        type->inheritance.reset(inheritance);
    }

    const RttiType* base = RttiType::Instance<B>();
    inheritance->baseTypes.emplace(RttiType::Instance<B>(), &RttiTypeDetail::CastFromTo<T, B>);
    return true;
}

template <typename T, typename D>
bool RttiInheritance::AddDerivedType()
{
    const RttiType* type = RttiType::Instance<T>();
    const RttiInheritance* inheritance = type->GetInheritance();

    if (nullptr == inheritance)
    {
        inheritance = new RttiInheritance();
        type->inheritance.reset(inheritance);
    }

    const RttiType* derived = RttiType::Instance<D>();
    inheritance->derivedTypes.emplace(derived, &RttiTypeDetail::CastFromTo<T, D>);
    return true;
}
} // namespace DAVA
