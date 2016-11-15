#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
class RtType;
class ReflectedStructure;
class DtorWrapper;
class CtorWrapper;
class StructureWrapper;

class ReflectedType final
{
    template <typename T>
    friend class ReflectionRegistrator;

    friend class ReflectedTypeDB;

public:
    ~ReflectedType();

    const RtType* GetRtType() const;
    const String& GetPermanentName() const;

    const ReflectedStructure* GetStrucutre() const;
    const StructureWrapper* GetStrucutreWrapper() const;

    Vector<const CtorWrapper*> GetCtors() const;
    const DtorWrapper* GetDtor() const;

    template <typename... Args>
    bool HasCtor(ReflectionCtorPolicy policy) const;

    bool HasDtor() const;

    template <typename... Args>
    Any Create(ReflectionCtorPolicy policy, Args...) const;

    void Destroy(Any&& any) const;

protected:
    String permanentName;
    const RtType* rtType;

    std::unique_ptr<ReflectedStructure> structure;
    std::unique_ptr<StructureWrapper> structureWrapper;

    ReflectedType(const RtType* rttiType_);
};
} // namespace DAVA
