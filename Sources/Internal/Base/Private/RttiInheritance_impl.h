#pragma once

#ifndef __Dava_RttiInheritance__
#include "Base/RttiInheritance.h"
#endif

namespace DAVA
{
namespace RttiInheritanceDetail
{
template <typename From, typename To>
void* CastFromTo(void* p)
{
    From* from = static_cast<From*>(p);
    To* to = static_cast<To*>(from);
    return to;
}
} // RttiInheritanceDetail

inline const Vector<RttiInheritance::Info>& RttiInheritance::GetBaseTypes() const
{
    return baseTypesInfo;
}

inline const Vector<RttiInheritance::Info>& RttiInheritance::GetDerivedTypes() const
{
    return derivedTypesInfo;
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
    inheritance->baseTypesInfo.push_back({ base, &RttiInheritanceDetail::CastFromTo<T, B> });
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
    inheritance->derivedTypesInfo.push_back({ derived, &RttiInheritanceDetail::CastFromTo<T, D> });
    return true;
}
} // namespace DAVA
