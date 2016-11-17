#pragma once

#ifndef __Dava_RtType__
#include "Base/Type.h"
#endif

namespace DAVA
{
namespace RttiTypeDetail
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

template <typename T>
struct TypeHolder
{
    static const Type* type;
};

template <typename T>
const Type* TypeHolder<T>::type = nullptr;
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
    if (nullptr != decayType)
        return decayType;

    return this;
}

inline const Type* Type::Deref() const
{
    return derefType;
}

inline const Type* Type::Pointer() const
{
    return pointerType;
}

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

    type.size = RttiTypeDetail::TypeSize<T>::size;
    type.name = typeid(T).name();
    type.stdTypeInfo = &typeid(T);

    type.flags.set(isConst, std::is_const<T>::value);
    type.flags.set(isPointer, std::is_pointer<T>::value);
    type.flags.set(isReference, std::is_reference<T>::value);
    type.flags.set(isFundamental, std::is_fundamental<T>::value);
    type.flags.set(isTrivial, std::is_trivial<T>::value);
    type.flags.set(isIntegral, std::is_integral<T>::value);
    type.flags.set(isFloatingPoint, std::is_floating_point<T>::value);
    type.flags.set(isEnum, std::is_enum<T>::value);

    auto condDeref = std::integral_constant<bool, needDeref>();
    type.derefType = RttiTypeDetail::GetTypeIfTrue<DerefU>(condDeref);

    auto condDecay = std::integral_constant<bool, needDecay>();
    type.decayType = RttiTypeDetail::GetTypeIfTrue<DecayU>(condDecay);

    auto condPointer = std::integral_constant<bool, needPointer>();
    type.pointerType = RttiTypeDetail::GetTypeIfTrue<PointerU>(condPointer);

    RttiTypeDetail::TypeHolder<T>::type = &type;
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

} // namespace DAVA
