#pragma once

#include <memory>
#include <typeindex>
#include <type_traits>
#include <bitset>

namespace DAVA
{
namespace TypeDetail
{
template <typename T>
struct TypeHolder;
}

class TypeInheritance;
class Type final
{
    friend class TypeInheritance;

    template <typename T>
    friend struct TypeDetail::TypeHolder;

public:
    struct Seed;

    template <typename T>
    using DecayT = std::conditional_t<std::is_pointer<T>::value, std::add_pointer_t<std::decay_t<std::remove_pointer_t<T>>>, std::decay_t<T>>;

    template <typename T>
    using DerefT = std::remove_pointer_t<std::decay_t<T>>;

    template <typename T>
    using PointerT = std::add_pointer_t<std::decay_t<T>>;

    using SeedCastOP = const Type::Seed* (*)(const void*);

    Type(Type&&) = delete;
    Type(const Type&) = delete;
    Type& operator=(const Type&) = delete;

    size_t GetSize() const;
    const char* GetName() const;
    std::type_index GetTypeIndex() const;
    const TypeInheritance* GetInheritance() const;
    unsigned long GetTypeFlags() const;
    SeedCastOP GetSeedCastOP() const;

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
    enum eTypeFlag
    {
        isConst,
        isPointer,
        isPointerToConst,
        isReference,
        isReferenceToConst,
        isFundamental,
        isTrivial,
        isIntegral,
        isFloatingPoint,
        isEnum
    };

    size_t size = 0;
    const char* name = nullptr;
    const std::type_info* stdTypeInfo = &typeid(void);
    SeedCastOP seedCastOP = nullptr;

    Type* const* derefType = nullptr;
    Type* const* decayType = nullptr;
    Type* const* pointerType = nullptr;

    std::bitset<sizeof(int) * 8> flags;
    std::unique_ptr<TypeInheritance, void (*)(TypeInheritance*)> inheritance;

    Type();
    TypeInheritance* EditInheritance() const;

    template <typename T>
    static Type* Init();
};

} // namespace DAVA

#define __Dava_Type__
#include "Base/Private/Type_impl.h"
