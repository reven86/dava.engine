#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
class Type;
class ReflectedStructure;
class StructureWrapper;

class ReflectedType final
{
    template <typename T>
    friend class ReflectionRegistrator;

    friend class ReflectedTypeDB;

public:
    enum class CreatePolicy
    {
        ByValue,
        ByPointer
    };

    ~ReflectedType();

    const Type* GetType() const;
    const String& GetPermanentName() const;

    const ReflectedStructure* GetStrucutre() const;
    const StructureWrapper* GetStrucutreWrapper() const;

    Vector<const AnyFn*> GetCtors() const;

    template <typename... Args>
    const AnyFn* GetCtor(const Type* retType = nullptr) const;

    const AnyFn* GetDtor() const;

    template <typename... Args>
    Any Create(CreatePolicy policy, Args... args) const;

    void Destroy(Any&& v) const;

protected:
    String permanentName;
    const Type* type;

    std::unique_ptr<ReflectedStructure> structure;
    std::unique_ptr<StructureWrapper> structureWrapper;

    ReflectedType(const Type* rttiType_);
};
} // namespace DAVA
