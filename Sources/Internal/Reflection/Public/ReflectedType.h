#pragma once
#define __DAVA_ReflectedType__

#include <memory>
#include "Base/Type.h"

namespace DAVA
{
class CtorWrapper;
class DtorWrapper;
class StructureWrapper;
class StructureEditorWrapper;
class ReflectedObject;

class ReflectedType final
{
    template <typename T>
    friend class ReflectionRegistrator;

public:
    const Type* GetType() const;

    const String& GetName() const;
    void SetName(const String&) const;

    template <typename T>
    static const ReflectedType* Get();

    template <typename T>
    static const ReflectedType* GetByPointer(const T* ptr);

    static const ReflectedType* GetByType(const Type* type);
    static const ReflectedType* GetByName(const String& name);

    template <typename T, typename... Bases>
    static void RegisterBases();

protected:
    mutable String name;
    //const Type* type = nullptr;

    Set<std::unique_ptr<CtorWrapper>> ctorWrappers;
    std::unique_ptr<DtorWrapper> dtorWrapper;
    std::unique_ptr<StructureWrapper> structureWrapper;
    std::unique_ptr<StructureEditorWrapper> structureEditorWrapper;

    ReflectedType() = default;

    template <typename T>
    static ReflectedType* Edit();

    template <typename T>
    static ReflectedType* Create();

    static UnorderedMap<const Type*, ReflectedType*> typeToReflectedTypeMap;
    static UnorderedMap<String, ReflectedType*> nameToReflectedTypeMap;
};

} // namespace DAVA

#define __DAVA_ReflectedType__
#include "Reflection/Private/ReflectedType_impl.h"
