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

inline const TypeInheritance* Type::GetInheritance() const
{
    return inheritance.get();
}

inline bool Type::IsConst() const
{
    return flags.test(static_cast<size_t>(TypeFlag::isConst));
}

inline bool Type::IsPointer() const
{
    return flags.test(static_cast<size_t>(TypeFlag::isPointer));
}

inline bool Type::IsReference() const
{
    return flags.test(static_cast<size_t>(TypeFlag::isReference));
}

inline bool Type::IsFundamental() const
{
    return flags.test(static_cast<size_t>(TypeFlag::isFundamental));
}

inline bool Type::IsTriviallyCopyable() const
{
    return flags.test(static_cast<size_t>(TypeFlag::isTriviallyCopyable));
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

inline const TypeInheritance::InheritanceMap& TypeInheritance::GetBaseTypes() const
{
    return baseTypes;
}

inline const TypeInheritance::InheritanceMap& TypeInheritance::GetDerivedTypes() const
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

    using DerefU = DerefT<T>;
    using DecayU = DecayT<T>;
    using PointerU = PointerT<T>;

    static const bool needDeref = (!std::is_same<T, DerefU>::value && !std::is_same<T, void*>::value);
    static const bool needDecay = (!std::is_same<T, DecayU>::value);
    static const bool needPointer = (!std::is_pointer<T>::value);

    type.name = typeid(T).name();
    type.size = TypeDetail::TypeSize<T>::size;

    type.flags.set(isConst, std::is_const<T>::value);
    type.flags.set(isPointer, std::is_pointer<T>::value);
    type.flags.set(isReference, std::is_reference<T>::value);
    type.flags.set(isFundamental, std::is_fundamental<T>::value);
    type.flags.set(isTriviallyCopyable, std::is_trivially_copyable<T>::value);

    auto condDeref = std::integral_constant<bool, needDeref>();
    type.derefType = TypeDetail::GetTypeIfTrue<DerefU>(condDeref);

    auto condDecay = std::integral_constant<bool, needDecay>();
    type.decayType = TypeDetail::GetTypeIfTrue<DecayU>(condDecay);

    auto condPointer = std::integral_constant<bool, needPointer>();
    type.pointerType = TypeDetail::GetTypeIfTrue<PointerU>(condPointer);

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
void TypeInheritance::RegisterBases()
{
    const Type* type = Type::Instance<T>();

    bool basesUnpack[] = { false, TypeInheritance::AddBaseType<T, Bases>()... };
    bool derivedUnpack[] = { false, TypeInheritance::AddDerivedType<Bases, T>()... };
}

template <typename T, typename B>
bool TypeInheritance::AddBaseType()
{
    const Type* type = Type::Instance<T>();
    if (type->inheritance == nullptr)
    {
        type->inheritance.reset(new TypeInheritance());
    }

    const Type* base = Type::Instance<B>();
    type->inheritance->baseTypes.emplace(Type::Instance<B>(), &TypeDetail::CastFromTo<T, B>);
    return true;
}

template <typename T, typename D>
bool TypeInheritance::AddDerivedType()
{
    const Type* type = Type::Instance<T>();
    if (type->inheritance == nullptr)
    {
        type->inheritance.reset(new TypeInheritance());
    }

    const Type* derived = Type::Instance<D>();
    type->inheritance->derivedTypes.emplace(derived, &TypeDetail::CastFromTo<T, D>);
    return true;
}

} // namespace DAVA
