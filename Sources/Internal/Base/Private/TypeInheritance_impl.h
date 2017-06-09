#pragma once

#ifndef __Dava_TypeInheritance__
#include "Base/TypeInheritance.h"
#endif

namespace DAVA
{
namespace TypeInheritanceDetail
{
static uint32_t stubData = 0xcccccccc;

template <typename From, typename To>
std::ptrdiff_t GetPtrDiff()
{
    const From* from = reinterpret_cast<const From*>(&stubData);
    const To* to = static_cast<const To*>(from);

    ptrdiff_t ret = reinterpret_cast<uintptr_t>(to) - reinterpret_cast<uintptr_t>(from);
    return ret;
}
} // namespace TypeInheritanceDetail

inline const Vector<TypeInheritance::Info>& TypeInheritance::GetBaseTypes() const
{
    return baseTypesInfo;
}

inline const Vector<TypeInheritance::Info>& TypeInheritance::GetDerivedTypes() const
{
    return derivedTypesInfo;
}

inline bool TypeInheritance::CanUpCast(const Type* from, const Type* to)
{
    void* out = nullptr;
    return TryCast(from, to, CastType::UpCast, nullptr, &out);
}

inline bool TypeInheritance::CanDownCast(const Type* from, const Type* to)
{
    void* out = nullptr;
    return TryCast(from, to, CastType::DownCast, nullptr, &out);
}

inline bool TypeInheritance::UpCast(const Type* from, const Type* to, void* inPtr, void** outPtr)
{
    return TryCast(from, to, CastType::UpCast, inPtr, outPtr);
}

inline bool TypeInheritance::DownCast(const Type* from, const Type* to, void* inPtr, void** outPtr)
{
    return TryCast(from, to, CastType::DownCast, inPtr, outPtr);
}

template <typename T, typename... Bases>
void TypeInheritance::RegisterBases()
{
    bool basesUnpack[] = { false, TypeInheritance::AddBaseType<T, Bases>()... };
    bool derivedUnpack[] = { false, TypeInheritance::AddDerivedType<Bases, T>()... };
}

template <typename T, typename B>
bool TypeInheritance::AddBaseType()
{
    const Type* type = Type::Instance<T>();
    const TypeInheritance* inheritance = type->GetInheritance();

    if (nullptr == inheritance)
    {
        Type* editType = const_cast<Type*>(type);

        inheritance = new TypeInheritance();
        editType->inheritance.reset(inheritance);
    }

    const Type* base = Type::Instance<B>();
    inheritance->baseTypesInfo.push_back({ base, TypeInheritanceDetail::GetPtrDiff<T, B>() });
    return true;
}

template <typename T, typename D>
bool TypeInheritance::AddDerivedType()
{
    const Type* type = Type::Instance<T>();
    const TypeInheritance* inheritance = type->GetInheritance();

    if (nullptr == inheritance)
    {
        Type* editType = const_cast<Type*>(type);

        inheritance = new TypeInheritance();
        editType->inheritance.reset(inheritance);
    }

    const Type* derived = Type::Instance<D>();
    inheritance->derivedTypesInfo.push_back({ derived, TypeInheritanceDetail::GetPtrDiff<T, D>() });
    return true;
}
} // namespace DAVA
