#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
class Type;
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

    const Type* GetType() const;
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
    const Type* type;

    std::unique_ptr<ReflectedStructure> structure;
    std::unique_ptr<StructureWrapper> structureWrapper;

    ReflectedType(const Type* rttiType_);
};
} // namespace DAVA
