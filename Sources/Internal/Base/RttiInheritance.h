#pragma once
#include "Base/BaseTypes.h"
#include "Base/RttiType.h"

namespace DAVA
{
class RttiInheritance final
{
public:
    using CastOP = void* (*)(void*);
    using InheritanceMap = UnorderedMap<const RttiType*, CastOP>;

    const InheritanceMap& GetBaseTypes() const;
    const InheritanceMap& GetDerivedTypes() const;

    template <typename T, typename... Bases>
    static void RegisterBases();

    static bool CanUpCast(const RttiType* from, const RttiType* to);
    static bool CanDownCast(const RttiType* from, const RttiType* to);
    static bool CanCast(const RttiType* from, const RttiType* to);

    static bool UpCast(const RttiType* from, void* inPtr, const RttiType* to, void** outPtr);
    static bool DownCast(const RttiType* from, void* inPtr, const RttiType* to, void** outPtr);
    static bool Cast(const RttiType* from, void* inPtr, const RttiType* to, void** outPtr);

private:
    mutable InheritanceMap baseTypes;
    mutable InheritanceMap derivedTypes;

    template <typename T, typename B>
    static bool AddBaseType();

    template <typename T, typename D>
    static bool AddDerivedType();
};
} // namespace DAVA

#define __Dava_RttiInheritance__
#include "Base/Private/RttiInheritance_impl.h"
