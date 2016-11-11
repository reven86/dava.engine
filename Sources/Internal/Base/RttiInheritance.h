#pragma once

#include "Base/BaseTypes.h"
#include "Base/RttiType.h"

namespace DAVA
{
class RttiInheritance final
{
public:
    struct Info
    {
        const RttiType* type;
        std::ptrdiff_t ptrDiff;
    };

    const Vector<Info>& GetBaseTypes() const;
    const Vector<Info>& GetDerivedTypes() const;

    template <typename T, typename... Bases>
    static void RegisterBases();

    static bool CanUpCast(const RttiType* from, const RttiType* to);
    static bool CanDownCast(const RttiType* from, const RttiType* to);
    static bool CanCast(const RttiType* from, const RttiType* to);

    static bool UpCast(const RttiType* from, const RttiType* to, void* inPtr, void** outPtr);
    static bool DownCast(const RttiType* from, const RttiType* to, void* inPtr, void** outPtr);
    static bool Cast(const RttiType* from, const RttiType* to, void* inPtr, void** outPtr);

private:
    enum class CastType
    {
        UpCast, //!< from class to derived class
        DownCast //!< from class to base class
    };

    mutable Vector<Info> baseTypesInfo;
    mutable Vector<Info> derivedTypesInfo;

    static bool TryCast(const RttiType* from, const RttiType* to, CastType castType, void* inPtr, void** outPtr);

    template <typename T, typename B>
    static bool AddBaseType();

    template <typename T, typename D>
    static bool AddDerivedType();
};
} // namespace DAVA

#define __Dava_RttiInheritance__
#include "Base/Private/RttiInheritance_impl.h"
