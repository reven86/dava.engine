#pragma once

#include "Base/BaseTypes.h"
#include "Base/RtType.h"

namespace DAVA
{
class RtTypeInheritance final
{
public:
    struct Info
    {
        const RtType* type;
        std::ptrdiff_t ptrDiff;
    };

    const Vector<Info>& GetBaseTypes() const;
    const Vector<Info>& GetDerivedTypes() const;

    template <typename T, typename... Bases>
    static void RegisterBases();

    static bool CanUpCast(const RtType* from, const RtType* to);
    static bool CanDownCast(const RtType* from, const RtType* to);
    static bool CanCast(const RtType* from, const RtType* to);

    static bool UpCast(const RtType* from, const RtType* to, void* inPtr, void** outPtr);
    static bool DownCast(const RtType* from, const RtType* to, void* inPtr, void** outPtr);
    static bool Cast(const RtType* from, const RtType* to, void* inPtr, void** outPtr);

private:
    enum class CastType
    {
        UpCast, //!< from class to derived class
        DownCast //!< from class to base class
    };

    mutable Vector<Info> baseTypesInfo;
    mutable Vector<Info> derivedTypesInfo;

    static bool TryCast(const RtType* from, const RtType* to, CastType castType, void* inPtr, void** outPtr);

    template <typename T, typename B>
    static bool AddBaseType();

    template <typename T, typename D>
    static bool AddDerivedType();
};
} // namespace DAVA

#define __Dava_RtTypeInheritance__
#include "Base/Private/RtTypeInheritance_impl.h"
