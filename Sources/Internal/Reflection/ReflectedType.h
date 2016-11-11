#pragma once

#include <memory>
#include "Base/BaseTypes.h"
#include "Reflection/Wrappers.h"

namespace DAVA
{
class RttiType;
class StructureWrapper;
class ReflectedStructure;

class ReflectedType final
{
    template <typename T>
    friend class ReflectionQualifier;

    friend class ReflectedTypeDB;

public:
    ~ReflectedType();

    const RttiType* GetRttiType() const;
    const String& GetPermanentName() const;

    const ReflectedStructure* GetStrucutre() const;
    const StructureWrapper* GetStrucutreWrapper() const;

    Vector<const CtorWrapper*> GetCtors() const;
    const DtorWrapper* GetDtor() const;

    template <typename... Args>
    bool HasCtor(CtorWrapper::Policy policy) const;

    bool HasDtor() const;

    template <typename... Args>
    Any Create(CtorWrapper::Policy policy, Args...) const;

    bool Destroy(Any&& any) const;

protected:
    String permanentName;
    const RttiType* rttiType;

    std::unique_ptr<ReflectedStructure> structure;
    std::unique_ptr<StructureWrapper> structureWrapper;

    ReflectedType(const RttiType* rttiType_);
};

} // namespace DAVA

#define __DAVA_ReflectedType__
#include "Reflection/Private/ReflectedType_impl.h"
