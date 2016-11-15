#pragma once

#include <memory>
#include <typeindex>
#include <type_traits>
#include <bitset>

#include "Base/BaseTypes.h"

namespace DAVA
{
class RtTypeInheritance;
class RtType final
{
    friend class RtTypeInheritance;

public:
    template <typename T>
    using DecayT = std::conditional_t<std::is_pointer<T>::value, std::add_pointer_t<std::decay_t<std::remove_pointer_t<T>>>, std::decay_t<T>>;

    template <typename T>
    using DerefT = std::remove_pointer_t<std::decay_t<T>>;

    template <typename T>
    using PointerT = std::add_pointer_t<std::decay_t<T>>;

    RtType(RtType&&) = delete;
    RtType(const RtType&) = delete;
    RtType& operator=(const RtType&) = delete;

    size_t GetSize() const;
    const char* GetName() const;
    std::type_index GetTypeIndex() const;
    const RtTypeInheritance* GetInheritance() const;

    bool IsConst() const;
    bool IsPointer() const;
    bool IsReference() const;
    bool IsFundamental() const;
    bool IsTrivial() const;
    bool IsEnum() const;

    const RtType* Decay() const;
    const RtType* Deref() const;
    const RtType* Pointer() const;

    template <typename T>
    static const RtType* Instance();

private:
    enum TypeFlag
    {
        isConst,
        isPointer,
        isReference,
        isFundamental,
        isTrivial,
        isEnum
    };

    size_t size = 0;
    const char* name;
    const std::type_info* stdTypeInfo = &typeid(void);

    const RtType* derefType = nullptr;
    const RtType* decayType = nullptr;
    const RtType* pointerType = nullptr;

    std::bitset<sizeof(int) * 8> flags;
    mutable std::unique_ptr<const RtTypeInheritance, void (*)(const RtTypeInheritance*)> inheritance;

    template <typename T>
    static void Init(RtType** ptype);

    RtType();
};

} // namespace DAVA

#define __Dava_RtType__
#include "Base/Private/RtType_impl.h"
