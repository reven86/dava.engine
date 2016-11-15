#pragma once

#ifndef __Dava_RtTypeInheritance__
#include "Base/RtTypeInheritance.h"
#endif

namespace DAVA
{
namespace RttiInheritanceDetail
{
template <typename From, typename To>
std::ptrdiff_t GetPtrDiff()
{
    From* from = reinterpret_cast<From*>(0xcccccccc);
    To* to = static_cast<To*>(from);

    ptrdiff_t ret = reinterpret_cast<uintptr_t>(to) - reinterpret_cast<uintptr_t>(from);

    if (0 != ret)
    {
        printf("1");
    }

    return ret;
}
} // RttiInheritanceDetail

inline const Vector<RtTypeInheritance::Info>& RtTypeInheritance::GetBaseTypes() const
{
    return baseTypesInfo;
}

inline const Vector<RtTypeInheritance::Info>& RtTypeInheritance::GetDerivedTypes() const
{
    return derivedTypesInfo;
}

inline bool RtTypeInheritance::CanUpCast(const RtType* from, const RtType* to)
{
    void* out = nullptr;
    return TryCast(from, to, CastType::UpCast, nullptr, &out);
}

inline bool RtTypeInheritance::CanDownCast(const RtType* from, const RtType* to)
{
    void* out = nullptr;
    return TryCast(from, to, CastType::DownCast, nullptr, &out);
}

inline bool RtTypeInheritance::UpCast(const RtType* from, const RtType* to, void* inPtr, void** outPtr)
{
    return TryCast(from, to, CastType::UpCast, inPtr, outPtr);
}

inline bool RtTypeInheritance::DownCast(const RtType* from, const RtType* to, void* inPtr, void** outPtr)
{
    return TryCast(from, to, CastType::DownCast, inPtr, outPtr);
}

template <typename T, typename... Bases>
void RtTypeInheritance::RegisterBases()
{
    const RtType* type = RtType::Instance<T>();

    bool basesUnpack[] = { false, RtTypeInheritance::AddBaseType<T, Bases>()... };
    bool derivedUnpack[] = { false, RtTypeInheritance::AddDerivedType<Bases, T>()... };
}

template <typename T, typename B>
bool RtTypeInheritance::AddBaseType()
{
    const RtType* type = RtType::Instance<T>();
    const RtTypeInheritance* inheritance = type->GetInheritance();

    if (nullptr == inheritance)
    {
        inheritance = new RtTypeInheritance();
        type->inheritance.reset(inheritance);
    }

    const RtType* base = RtType::Instance<B>();
    inheritance->baseTypesInfo.push_back({ base, RttiInheritanceDetail::GetPtrDiff<T, B>() });
    return true;
}

template <typename T, typename D>
bool RtTypeInheritance::AddDerivedType()
{
    const RtType* type = RtType::Instance<T>();
    const RtTypeInheritance* inheritance = type->GetInheritance();

    if (nullptr == inheritance)
    {
        inheritance = new RtTypeInheritance();
        type->inheritance.reset(inheritance);
    }

    const RtType* derived = RtType::Instance<D>();
    inheritance->derivedTypesInfo.push_back({ derived, RttiInheritanceDetail::GetPtrDiff<T, D>() });
    return true;
}
} // namespace DAVA
