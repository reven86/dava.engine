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
    const TypeInheritance* GetInheritance() const;

    bool IsConst() const;
    bool IsPointer() const;
    bool IsReference() const;
    bool IsFundamental() const;
    bool IsTrivial() const;

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
    };

    size_t size = 0;
    const char* name = nullptr;

    const Type* derefType = nullptr;
    const Type* decayType = nullptr;
    const Type* pointerType = nullptr;

    std::bitset<sizeof(int) * 8> flags;
    mutable std::unique_ptr<TypeInheritance> inheritance;

    Type() = default;

    template <typename T>
    static void Init(Type** ptype);
};

class TypeInheritance final
{
public:
    using CastOP = void* (*)(void*);
    using InheritanceMap = UnorderedMap<const Type*, CastOP>;

    const InheritanceMap& GetBaseTypes() const;
    const InheritanceMap& GetDerivedTypes() const;

    template <typename T, typename... Bases>
    static void RegisterBases();

    static bool CanUpCast(const Type* from, const Type* to);
    static bool CanDownCast(const Type* from, const Type* to);
    static bool CanCast(const Type* from, const Type* to);

    static bool UpCast(const Type* from, void* inPtr, const Type* to, void** outPtr);
    static bool DownCast(const Type* from, void* inPtr, const Type* to, void** outPtr);
    static bool Cast(const Type* from, void* inPtr, const Type* to, void** outPtr);

private:
    InheritanceMap baseTypes;
    InheritanceMap derivedTypes;

    template <typename T, typename B>
    static bool AddBaseType();

    template <typename T, typename D>
    static bool AddDerivedType();
};

} // namespace DAVA

#define __Dava_Type__
#include "Base/Private/Type_impl.h"
