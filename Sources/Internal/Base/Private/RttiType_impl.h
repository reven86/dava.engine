#pragma once

#ifndef __Dava_RttiType__
#include "Base/RttiType.h"
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
const RttiType* GetTypeIfTrue(std::false_type)
{
    return nullptr;
}

template <typename T>
const RttiType* GetTypeIfTrue(std::true_type)
{
    return RttiType::Instance<T>();
}

template <typename T>
struct TypeHolder
{
    static const RttiType* rttiType;
};

template <typename T>
const RttiType* TypeHolder<T>::rttiType = nullptr;
} // namespace TypeDetails

inline size_t RttiType::GetSize() const
{
    return size;
}

inline const char* RttiType::GetName() const
{
    return stdTypeInfo->name();
}

inline std::type_index RttiType::GetTypeIndex() const
{
    return std::type_index(*stdTypeInfo);
}

inline const RttiInheritance* RttiType::GetInheritance() const
{
    return static_cast<const RttiInheritance*>(inheritance.get());
}

inline bool RttiType::IsConst() const
{
    return flags.test(static_cast<size_t>(TypeFlag::isConst));
}

inline bool RttiType::IsPointer() const
{
    return flags.test(static_cast<size_t>(TypeFlag::isPointer));
}

inline bool RttiType::IsReference() const
{
    return flags.test(static_cast<size_t>(TypeFlag::isReference));
}

inline bool RttiType::IsFundamental() const
{
    return flags.test(static_cast<size_t>(TypeFlag::isFundamental));
}

inline bool RttiType::IsTrivial() const
{
    return flags.test(static_cast<size_t>(TypeFlag::isTrivial));
}

inline bool RttiType::IsEnum() const
{
    return flags.test(static_cast<size_t>(TypeFlag::isEnum));
}

inline const RttiType* RttiType::Decay() const
{
    return decayType;
}

inline const RttiType* RttiType::Deref() const
{
    return derefType;
}

inline const RttiType* RttiType::Pointer() const
{
    return pointerType;
}

template <typename T>
void RttiType::Init(RttiType** ptype)
{
    static RttiType rttiType;

    *ptype = &rttiType;

    using DerefU = DerefT<T>;
    using DecayU = DecayT<T>;
    using PointerU = PointerT<T>;

    static const bool needDeref = (!std::is_same<T, DerefU>::value && !std::is_same<T, void*>::value);
    static const bool needDecay = (!std::is_same<T, DecayU>::value);
    static const bool needPointer = (!std::is_pointer<T>::value);

    rttiType.size = RttiTypeDetail::TypeSize<T>::size;
    rttiType.name = typeid(T).name();
    rttiType.stdTypeInfo = &typeid(T);

    rttiType.flags.set(isConst, std::is_const<T>::value);
    rttiType.flags.set(isPointer, std::is_pointer<T>::value);
    rttiType.flags.set(isReference, std::is_reference<T>::value);
    rttiType.flags.set(isFundamental, std::is_fundamental<T>::value);
    rttiType.flags.set(isTrivial, std::is_trivial<T>::value);
    rttiType.flags.set(isEnum, std::is_enum<T>::value);

    auto condDeref = std::integral_constant<bool, needDeref>();
    rttiType.derefType = RttiTypeDetail::GetTypeIfTrue<DerefU>(condDeref);

    auto condDecay = std::integral_constant<bool, needDecay>();
    rttiType.decayType = RttiTypeDetail::GetTypeIfTrue<DecayU>(condDecay);

    auto condPointer = std::integral_constant<bool, needPointer>();
    rttiType.pointerType = RttiTypeDetail::GetTypeIfTrue<PointerU>(condPointer);

    RttiTypeDetail::TypeHolder<T>::rttiType = &rttiType;
}

template <typename T>
const RttiType* RttiType::Instance()
{
    static RttiType* type = nullptr;

    if (nullptr == type)
    {
        RttiType::Init<T>(&type);
    }

    return type;
}

} // namespace DAVA
