#pragma once

#include <memory>
#include <typeindex>
#include <type_traits>
#include <bitset>

#include "Base/BaseTypes.h"

namespace DAVA
{
class RttiInheritance;
class RttiType final
{
    friend class RttiInheritance;

public:
    template <typename T>
    using DecayT = std::conditional_t<std::is_pointer<T>::value, std::add_pointer_t<std::decay_t<std::remove_pointer_t<T>>>, std::decay_t<T>>;

    template <typename T>
    using DerefT = std::remove_pointer_t<std::decay_t<T>>;

    template <typename T>
    using PointerT = std::add_pointer_t<std::decay_t<T>>;

    RttiType(RttiType&&) = delete;
    RttiType(const RttiType&) = delete;
    RttiType& operator=(const RttiType&) = delete;

    size_t GetSize() const;
    const char* GetName() const;
    std::type_index GetTypeIndex() const;
    const RttiInheritance* GetInheritance() const;

    bool IsConst() const;
    bool IsPointer() const;
    bool IsReference() const;
    bool IsFundamental() const;
    bool IsTrivial() const;
    bool IsEnum() const;

    const RttiType* Decay() const;
    const RttiType* Deref() const;
    const RttiType* Pointer() const;

    template <typename T>
    static const RttiType* Instance();

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

    const RttiType* derefType = nullptr;
    const RttiType* decayType = nullptr;
    const RttiType* pointerType = nullptr;

    std::bitset<sizeof(int) * 8> flags;
    mutable std::unique_ptr<const RttiInheritance, void (*)(const RttiInheritance*)> inheritance;

    template <typename T>
    static void Init(RttiType** ptype);

    RttiType();
};

} // namespace DAVA

#define __Dava_RttiType__
#include "Base/Private/RttiType_impl.h"
