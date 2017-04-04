#pragma once

#ifndef __Dava_Type__
#include "Base/Type.h"
#endif

#include "Base/List.h"

namespace DAVA
{
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

static List<Type**> allTypes;

template <typename T>
struct TypeHolder
{
    static Type* t;
    static Type* Instance()
    {
        return t;
    }
    static Type** InstancePointer()
    {
        return &t;
    }
};

template <typename T>
Type* TypeHolder<T>::t = Type::Init<T>();

static Type* nullType = nullptr;

template <typename T>
Type* const* GetTypeIfTrue(std::false_type)
{
    return &nullType;
}

template <typename T>
Type* const* GetTypeIfTrue(std::true_type)
{
    return TypeHolder<T>::InstancePointer();
}

} // namespace TypeDetails

inline size_t Type::GetSize() const
{
    return size;
}

inline const char* Type::GetName() const
{
    return stdTypeInfo->name();
}

inline std::type_index Type::GetTypeIndex() const
{
    return std::type_index(*stdTypeInfo);
}

inline const TypeInheritance* Type::GetInheritance() const
{
    return static_cast<const TypeInheritance*>(inheritance.get());
}

inline unsigned long Type::GetTypeFlags() const
{
    return flags.to_ulong();
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

inline bool Type::IsTrivial() const
{
    return flags.test(static_cast<size_t>(TypeFlag::isTrivial));
}

inline bool Type::IsIntegral() const
{
    return flags.test(static_cast<size_t>(TypeFlag::isIntegral));
}

inline bool Type::IsFloatingPoint() const
{
    return flags.test(static_cast<size_t>(TypeFlag::isFloatingPoint));
}

inline bool Type::IsEnum() const
{
    return flags.test(static_cast<size_t>(TypeFlag::isEnum));
}

inline const Type* Type::Decay() const
{
    const Type* decayedType = *decayType;

    if (nullptr != decayedType)
        return decayedType;

    return this;
}

inline const Type* Type::Deref() const
{
    return *derefType;
}

inline const Type* Type::Pointer() const
{
    return *pointerType;
}

template <typename T>
Type* Type::Init()
{
    static Type type;

    using DerefU = DerefT<T>;
    using DecayU = DecayT<T>;
    using PointerU = PointerT<T>;

    static const bool needDeref = (!std::is_same<T, DerefU>::value);
    static const bool needDecay = (!std::is_same<T, DecayU>::value);
    static const bool needPointer = (!std::is_pointer<T>::value);

    type.size = TypeDetail::TypeSize<T>::size;
    type.name = typeid(T).name();
    type.stdTypeInfo = &typeid(T);

    type.flags.set(isConst, std::is_const<T>::value);
    type.flags.set(isPointer, std::is_pointer<T>::value);
    type.flags.set(isPointerToConst, std::is_pointer<T>::value && std::is_const<typename std::remove_pointer<T>::type>::value);
    type.flags.set(isReference, std::is_reference<T>::value);
    type.flags.set(isReferenceToConst, std::is_reference<T>::value && std::is_const<typename std::remove_reference<T>::type>::value);
    type.flags.set(isFundamental, std::is_fundamental<T>::value);
    type.flags.set(isTrivial, std::is_trivial<T>::value);
    type.flags.set(isIntegral, std::is_integral<T>::value);
    type.flags.set(isFloatingPoint, std::is_floating_point<T>::value);
    type.flags.set(isEnum, std::is_enum<T>::value);

    auto condDeref = std::integral_constant<bool, needDeref>();
    type.derefType = TypeDetail::GetTypeIfTrue<DerefU>(condDeref);

    auto condDecay = std::integral_constant<bool, needDecay>();
    type.decayType = TypeDetail::GetTypeIfTrue<DecayU>(condDecay);

    auto condPointer = std::integral_constant<bool, needPointer>();
    type.pointerType = TypeDetail::GetTypeIfTrue<PointerU>(condPointer);

    //TypeDetail::allTypes.push_back(TypeDetail::TypeHolder<T>::InstancePointer());

    return &type;
}

template <typename T>
const Type* Type::Instance()
{
    return TypeDetail::TypeHolder<T>::Instance();
}

} // namespace DAVA
