#pragma once

#ifndef __Dava_RtType__
#include "Base/RtType.h"
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
const RtType* GetTypeIfTrue(std::false_type)
{
    return nullptr;
}

template <typename T>
const RtType* GetTypeIfTrue(std::true_type)
{
    return RtType::Instance<T>();
}

template <typename T>
struct TypeHolder
{
    static const RtType* rtType;
};

template <typename T>
const RtType* TypeHolder<T>::rtType = nullptr;
} // namespace TypeDetails

inline size_t RtType::GetSize() const
{
    return size;
}

inline const char* RtType::GetName() const
{
    return stdTypeInfo->name();
}

inline std::type_index RtType::GetTypeIndex() const
{
    return std::type_index(*stdTypeInfo);
}

inline const RtTypeInheritance* RtType::GetInheritance() const
{
    return static_cast<const RtTypeInheritance*>(inheritance.get());
}

inline bool RtType::IsConst() const
{
    return flags.test(static_cast<size_t>(TypeFlag::isConst));
}

inline bool RtType::IsPointer() const
{
    return flags.test(static_cast<size_t>(TypeFlag::isPointer));
}

inline bool RtType::IsReference() const
{
    return flags.test(static_cast<size_t>(TypeFlag::isReference));
}

inline bool RtType::IsFundamental() const
{
    return flags.test(static_cast<size_t>(TypeFlag::isFundamental));
}

inline bool RtType::IsTrivial() const
{
    return flags.test(static_cast<size_t>(TypeFlag::isTrivial));
}

inline bool RtType::IsEnum() const
{
    return flags.test(static_cast<size_t>(TypeFlag::isEnum));
}

inline const RtType* RtType::Decay() const
{
    if (nullptr != decayType)
        return decayType;

    return this;
}

inline const RtType* RtType::Deref() const
{
    return derefType;
}

inline const RtType* RtType::Pointer() const
{
    return pointerType;
}

template <typename T>
void RtType::Init(RtType** ptype)
{
    static RtType rtType;

    *ptype = &rtType;

    using DerefU = DerefT<T>;
    using DecayU = DecayT<T>;
    using PointerU = PointerT<T>;

    static const bool needDeref = (!std::is_same<T, DerefU>::value && !std::is_same<T, void*>::value);
    static const bool needDecay = (!std::is_same<T, DecayU>::value);
    static const bool needPointer = (!std::is_pointer<T>::value);

    rtType.size = RttiTypeDetail::TypeSize<T>::size;
    rtType.name = typeid(T).name();
    rtType.stdTypeInfo = &typeid(T);

    rtType.flags.set(isConst, std::is_const<T>::value);
    rtType.flags.set(isPointer, std::is_pointer<T>::value);
    rtType.flags.set(isReference, std::is_reference<T>::value);
    rtType.flags.set(isFundamental, std::is_fundamental<T>::value);
    rtType.flags.set(isTrivial, std::is_trivial<T>::value);
    rtType.flags.set(isEnum, std::is_enum<T>::value);

    auto condDeref = std::integral_constant<bool, needDeref>();
    rtType.derefType = RttiTypeDetail::GetTypeIfTrue<DerefU>(condDeref);

    auto condDecay = std::integral_constant<bool, needDecay>();
    rtType.decayType = RttiTypeDetail::GetTypeIfTrue<DecayU>(condDecay);

    auto condPointer = std::integral_constant<bool, needPointer>();
    rtType.pointerType = RttiTypeDetail::GetTypeIfTrue<PointerU>(condPointer);

    RttiTypeDetail::TypeHolder<T>::rtType = &rtType;
}

template <typename T>
const RtType* RtType::Instance()
{
    static RtType* type = nullptr;

    if (nullptr == type)
    {
        RtType::Init<T>(&type);
    }

    return type;
}

} // namespace DAVA
