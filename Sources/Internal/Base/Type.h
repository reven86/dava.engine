#pragma once

#include <memory>
#include <typeindex>
#include <type_traits>
#include <bitset>

#include "Base/BaseTypes.h"

namespace DAVA
{
class TypeInheritance;
class Type final
{
    friend class TypeInheritance;

public:
    template <typename T>
    using DecayT = std::conditional_t<std::is_pointer<T>::value, std::add_pointer_t<std::decay_t<std::remove_pointer_t<T>>>, std::decay_t<T>>;

    template <typename T>
    using DerefT = std::remove_pointer_t<std::decay_t<T>>;

    template <typename T>
    using PointerT = std::add_pointer_t<std::decay_t<T>>;

    Type(Type&&) = delete;
    Type(const Type&) = delete;
    Type& operator=(const Type&) = delete;

    size_t GetSize() const;
    const char* GetName() const;
    std::type_index GetTypeIndex() const;
    const TypeInheritance* GetInheritance() const;

    bool IsConst() const;
    bool IsPointer() const;
    bool IsReference() const;
    bool IsFundamental() const;
    bool IsTrivial() const;
    bool IsIntegral() const;
    bool IsFloatingPoint() const;
    bool IsEnum() const;

    const Type* Decay() const;
    const Type* Deref() const;
    const Type* Pointer() const;

    template <typename T>
    static const Type* Instance();

private:
    enum TypeFlag
    {
        isConst,
        isPointer,
        isReference,
        isFundamental,
        isTrivial,
        isIntegral,
        isFloatingPoint,
        isEnum
    };

    size_t size = 0;
    const char* name = nullptr;
    const std::type_info* stdTypeInfo = &typeid(void);

    const Type* derefType = nullptr;
    const Type* decayType = nullptr;
    const Type* pointerType = nullptr;

    std::bitset<sizeof(int) * 8> flags;
    mutable std::unique_ptr<const TypeInheritance, void (*)(const TypeInheritance*)> inheritance;

    template <typename T>
    static void Init(Type** ptype);

    Type();
};

} // namespace DAVA

#define __Dava_Type__
#include "Base/Private/Type_impl.h"
