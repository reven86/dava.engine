#pragma once
#define DAVA_REFLECTION_DB__H

#include <memory>
#include <cassert>
#include <vector>
#include <unordered_map>

#include "Reflection/ReflectionVirt.h"
#include "Reflection/ReflectionWrappers.h"

namespace DAVA
{
class ReflectionDB final
{
    template <typename T>
    friend struct ReflectionRegistrator;

public:
    std::unique_ptr<StructureWrapper> structureWrapper;
    std::unique_ptr<DtorWrapper> dtorWrapper;
    DAVA::Vector<std::unique_ptr<CtorWrapper>> ctorWrappers;
    DAVA::UnorderedMultiMap<DAVA::String, std::unique_ptr<MethodWrapper>> methodWrappers;

    const DtorWrapper* GetDtor() const;
    const CtorWrapper* GetCtor() const;
    const CtorWrapper* GetCtor(const Ref::ParamsList& params) const;
    DAVA::Vector<const CtorWrapper*> GetCtors() const;

    template <typename T>
    static const ReflectionDB* GetGlobalDB();

    template <typename T>
    static const ReflectionDB* GetGlobalDB(T* object);

protected:
    template <typename T>
    static ReflectionDB* EditGlobalDB();

    template <typename B, typename D>
    static void RegisterBaseClass();
};

} // namespace DAVA

#include "Private/ReflectionDBImpl.h"
