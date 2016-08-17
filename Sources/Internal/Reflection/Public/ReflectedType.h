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

    const String& GetRttiName() const;
    const String& GetPermanentName() const;

    void SetPermanentName(const String&) const;

    // TODO:
    // move into private section
    // and add appropriate access methods
    //
    // -->
    //
    Set<std::unique_ptr<CtorWrapper>> ctorWrappers;
    std::unique_ptr<DtorWrapper> dtorWrapper;
    std::unique_ptr<StructureWrapper> structureWrapper;
    std::unique_ptr<StructureEditorWrapper> structureEditorWrapper;
    //
    // <--

    template <typename T>
    static const ReflectedType* Get();

    template <typename T>
    static const ReflectedType* GetByPointer(const T* ptr);

    static const ReflectedType* GetByType(const Type* type);
    static const ReflectedType* GetByRttiName(const String& name);
    static const ReflectedType* GetByPermanentName(const String& name);

    template <typename T, typename... Bases>
    static void RegisterBases();

protected:
    const Type* type = nullptr;

    String rttiName;
    String permanentName;

    ReflectedType() = default;

    template <typename T>
    static ReflectedType* Edit();

    template <typename T>
    static ReflectedType* Create();

    static UnorderedMap<const Type*, ReflectedType*> typeToReflectedTypeMap;
    static UnorderedMap<String, ReflectedType*> permanentNameToReflectedTypeMap;
    static UnorderedMap<String, ReflectedType*> rttiNameToReflectedTypeMap;
};

} // namespace DAVA

#define __DAVA_ReflectedType__
#include "Reflection/Private/ReflectedType_impl.h"
