#pragma once

#include <memory>
#include <typeindex>

#include "Base/BaseTypes.h"

namespace DAVA
{
class Type
{
public:
    using CastToBaseOP = void* (*)(void*);

    Type(Type&&) = delete;
    Type(const Type&) = delete;
    Type& operator=(const Type&) = delete;

    size_t GetSize() const;
    const char* GetName() const;

    bool IsConst() const;
    bool IsPointer() const;
    bool IsReference() const;
    bool IsDerivedFrom(const Type*) const;

    const Type* Decay() const;
    const Type* Deref() const;
    const Type* Pointer() const;

    const UnorderedMap<const Type*, CastToBaseOP>& GetBaseTypes() const;
    const UnorderedSet<const Type*> GetDerivedTypes() const;

    template <typename T>
    static const Type* Instance();

    template <typename T, typename... Bases>
    static void RegisterBases();

protected:
    Type() = default;

    template <typename T>
    static void Init(Type** ptype);

    size_t size = 0;
    const char* name = nullptr;

    bool isConst = false;
    bool isPointer = false;
    bool isReference = false;

    const Type* derefType = nullptr;
    const Type* decayType = nullptr;
    const Type* pointerType = nullptr;

    mutable UnorderedMap<const Type*, CastToBaseOP> baseTypes;
    mutable UnorderedSet<const Type*> derivedTypes;
};

} // namespace DAVA

#define __Dava_Type__
#include "Base/Private/Type_impl.h"
