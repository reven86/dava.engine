#pragma once

#include <memory>
#include <typeindex>
#include <type_traits>

#include "Base/BaseTypes.h"

namespace DAVA
{
class Type final
{
public:
    template <typename T>
    using DecayT = std::conditional_t<std::is_pointer<T>::value, std::add_pointer_t<std::decay_t<std::remove_pointer_t<T>>>, std::decay_t<T>>;

    template <typename T>
    using DerefT = std::remove_pointer_t<std::remove_reference_t<std::remove_cv_t<T>>>;

    template <typename T>
    using PointerT = std::add_pointer_t<std::decay_t<T>>;

    using InheritanceCastOP = void* (*)(void*);
    using InheritanceMap = UnorderedMap<const Type*, InheritanceCastOP>;

    Type(Type&&) = delete;
    Type(const Type&) = delete;
    Type& operator=(const Type&) = delete;

    size_t GetSize() const;
    const char* GetName() const;

    bool IsConst() const;
    bool IsPointer() const;
    bool IsReference() const;
    bool IsFundamental() const;

    const Type* Decay() const;
    const Type* Deref() const;
    const Type* Pointer() const;

    const InheritanceMap& BaseTypes() const;
    const InheritanceMap& DerivedTypes() const;

    template <typename T>
    static const Type* Instance();

    template <typename T, typename... Bases>
    static void RegisterBases();

private:
    size_t size = 0;
    const char* name = nullptr;

    bool isConst = false;
    bool isPointer = false;
    bool isReference = false;
    bool isFundamental = false;

    const Type* derefType = nullptr;
    const Type* decayType = nullptr;
    const Type* pointerType = nullptr;

    mutable InheritanceMap baseTypes;
    mutable InheritanceMap derivedTypes;

    Type() = default;

    template <typename T>
    static void Init(Type** ptype);

    template <typename T, typename B>
    static bool AddBaseType();

    template <typename T, typename D>
    static bool AddDerivedType();
};

struct TypeCast
{
    static bool CanUpCast(const Type* from, const Type* to);
    static bool CanDownCast(const Type* from, const Type* to);
    static bool CanCast(const Type* from, const Type* to);

    static bool UpCast(const Type* from, void* inPtr, const Type* to, void** outPtr);
    static bool DownCast(const Type* from, void* inPtr, const Type* to, void** outPtr);
    static bool Cast(const Type* from, void* inPtr, const Type* to, void** outPtr);
};

} // namespace DAVA

#define __Dava_Type__
#include "Base/Private/Type_impl.h"
