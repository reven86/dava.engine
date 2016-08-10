#pragma once

#ifndef __Dava_Type__
#include "Base/Type.h"
#endif

#include <atomic>

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

inline bool Type::IsDerivedFrom(const Type* type) const
{
    return (baseTypes.count(type) > 0);
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

inline const UnorderedMap<const Type*, Type::CastToBaseOP>& Type::GetBaseTypes() const
{
    return baseTypes;
}

inline const UnorderedSet<const Type*> Type::GetDerivedTypes() const
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

template <typename D, typename B>
void* CastToBase(void* p)
{
    D* derived = static_cast<D*>(p);
    B* base = static_cast<B*>(derived);
    return base;
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

    using DerefT = std::remove_pointer_t<std::remove_reference_t<std::remove_cv_t<T>>>;
    using DecayT = std::conditional_t<std::is_pointer<T>::value, std::add_pointer_t<std::decay_t<std::remove_pointer_t<T>>>, std::decay_t<T>>;
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
inline const Type* Type::Instance()
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

    bool basesUnpack[] = { false, type->baseTypes.insert(std::make_pair(Type::Instance<Bases>(), &TypeDetail::CastToBase<T, Bases>)).second... };
    bool derivedUnpack[] = { false, Type::Instance<Bases>()->derivedTypes.insert(type).second... };
}

} // namespace DAVA
