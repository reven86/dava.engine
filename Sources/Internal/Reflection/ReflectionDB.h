#pragma once
#define DAVA_REFLECTION_DB__H

#include "ReflectionVirt.h"
#include "ReflectionWrappers.h"

#include <set>
#include <memory>
#include <cassert>

namespace DAVA
{
class ReflectionDB final
{
    template <typename T>
    friend struct ReflectionRegistrator;

public:
    std::unique_ptr<StructureWrapper> structureWrapper;
    std::unique_ptr<DtorWrapper> dtorWrapper;
    std::vector<std::unique_ptr<CtorWrapper>> ctorWrappers;
    std::unordered_multimap<std::string, std::unique_ptr<MethodWrapper>> methodWrappers;

    template <typename T>
    static ReflectionDB* CreateDB();

    template <typename T>
    static const ReflectionDB* GetGlobalDB();

    template <typename T>
    static const ReflectionDB* GetGlobalDB(T* object);

protected:
    template <typename T>
    static ReflectionDB* EditGlobalDB();

    static std::set<std::unique_ptr<ReflectionDB>> allDBs;
};

} // namespace DAVA

#include "Private/ReflectionDBImpl.h"
