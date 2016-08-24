#pragma once

#ifndef __Dava_Type__
#include "Base/Type.h"
#endif

#include <atomic>
#include <functional>

namespace DAVA
{
inline size_t Type::GetSize() const
{
    return size;
}

inline const char* Type::GetName() const
{
    return name;
}

inline bool Type::IsConst() const
{
    return isConst;
}

inline bool Type::IsPointer() const
{
    return isPointer;
}

inline bool Type::IsReference() const
{
    return isReference;
}

inline const Type* Type::Decay() const
{
    return decayType;
}

inline const Type* Type::Deref() const
{
    return derefType;
}

inline const Type* Type::Pointer() const
{
    return pointerType;
}

inline const Type::InheritanceMap& Type::BaseTypes() const
{
    return baseTypes;
}

inline const Type::InheritanceMap& Type::DerivedTypes() const
{
    return derivedTypes;
}

namespace TypeDetail
{
template <typename T>
struct TypeSize
{
    static const size_t size = sizeof(T);
};

template <>
struct TypeSize<void>
{
    static const size_t size = 0;
};

template <>
struct TypeSize<const void>
{
    static const size_t size = 0;
};

template <typename T>
const Type* GetTypeIfTrue(std::false_type)
{
    return nullptr;
}

template <typename T>
const Type* GetTypeIfTrue(std::true_type)
{
    return Type::Instance<T>();
}

template <typename From, typename To>
void* CastFromTo(void* p)
{
    From* from = static_cast<From*>(p);
    To* to = static_cast<To*>(from);
    return to;
}

template <typename T>
struct TypeHolder
{
    static const Type* type;
};

template <typename T>
const Type* TypeHolder<T>::type = nullptr;

} // namespace TypeDetails

template <typename T>
void Type::Init(Type** ptype)
{
    static Type type;

    *ptype = &type;

    using DecayT = std::conditional_t<std::is_pointer<T>::value, std::add_pointer_t<std::decay_t<std::remove_pointer_t<T>>>, std::decay_t<T>>;
    using DerefT = std::remove_pointer_t<std::remove_reference_t<std::remove_cv_t<T>>>;
    using PointerT = std::add_pointer_t<std::decay_t<T>>;

    static const bool needDeref = (!std::is_same<T, DerefT>::value && !std::is_same<T, void*>::value);
    static const bool needDecay = (!std::is_same<T, DecayT>::value);
    static const bool needPointer = (!std::is_pointer<T>::value);

    type.name = typeid(T).name();
    type.size = TypeDetail::TypeSize<T>::size;

    type.isConst = std::is_const<T>::value;
    type.isPointer = std::is_pointer<T>::value;
    type.isReference = std::is_reference<T>::value;

    auto condDeref = std::integral_constant<bool, needDeref>();
    type.derefType = TypeDetail::GetTypeIfTrue<DerefT>(condDeref);

    auto condDecay = std::integral_constant<bool, needDecay>();
    type.decayType = TypeDetail::GetTypeIfTrue<DecayT>(condDecay);

    auto condPointer = std::integral_constant<bool, needPointer>();
    type.pointerType = TypeDetail::GetTypeIfTrue<PointerT>(condPointer);

    TypeDetail::TypeHolder<T>::type = &type;
}

template <typename T>
const Type* Type::Instance()
{
    static Type* type = nullptr;

    if (nullptr == type)
    {
        Type::Init<T>(&type);
    }

    return type;
}

template <typename T, typename... Bases>
void Type::RegisterBases()
{
    const Type* type = Type::Instance<T>();

    bool basesUnpack[] = { false, Type::AddBaseType<T, Bases>()... };
    bool derivedUnpack[] = { false, Type::AddDerivedType<Bases, T>()... };
}

template <typename T, typename B>
bool Type::AddBaseType()
{
    const Type* type = Type::Instance<T>();
    const Type* base = Type::Instance<B>();
    type->baseTypes.emplace(base, &TypeDetail::CastFromTo<T, B>);

    return true;
}

template <typename T, typename D>
bool Type::AddDerivedType()
{
    const Type* type = Type::Instance<T>();
    const Type* derived = Type::Instance<D>();
    type->derivedTypes.emplace(derived, &TypeDetail::CastFromTo<T, D>);

    return true;
}

} // namespace DAVA
