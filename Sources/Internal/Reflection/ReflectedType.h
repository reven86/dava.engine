#pragma once

#include <memory>
#include "Base/RttiType.h"
#include "Base/AnyFn.h"
#include "Reflection/Wrappers.h"

namespace DAVA
{
class ReflectedStructure final
{
public:
    struct Field
    {
        String name;
        std::unique_ptr<ReflectedMeta> meta;
        std::unique_ptr<FieldWrapper> fieldWrapper;

        const ReflectedType* rtype;
    };

    struct Method
    {
        String name;
        std::unique_ptr<ReflectedMeta> meta;
        std::unique_ptr<MethodWrapper> methodWrapper;
    };

    struct Enum
    {
        String name;
        std::unique_ptr<EnumWrapper> enumWrapper;
    };

    Vector<std::unique_ptr<CtorWrapper>> ctors;
    Vector<std::unique_ptr<DtorWrapper>> dtors;
    Vector<std::unique_ptr<Field>> fields;
    Vector<std::unique_ptr<Method>> methods;
    Vector<std::unique_ptr<Enum>> enums;
};

class ReflectedType final
{
    template <typename C>
    friend class ReflectionRegistrator;

public:
    const RttiType* GetRttiType();

    const String& GetPermanentName() const;
    void SetPermanentName(const String& name) const;

    const ReflectedMeta* GetMeta() const;
    const ReflectedStructure* GetStructure() const;
    const StructureWrapper* GetStructureWrapper() const;

    template <typename T>
    static const ReflectedType* Get();

    template <typename T>
    static const ReflectedType* GetByPointer(const T* ptr);

    static const ReflectedType* GetByType(const RttiType* type);
    static const ReflectedType* GetByRttiName(const String& name);
    static const ReflectedType* GetByPermanentName(const String& name);

    template <typename T, typename... Bases>
    static void RegisterBases();

protected:
    const RttiType* type;
    String permanentName;

    std::unique_ptr<ReflectedMeta> meta;
    std::unique_ptr<ReflectedStructure> structure;
    std::unique_ptr<StructureWrapper> structureWrapper;

    ReflectedType() = default;

    template <typename T>
    static ReflectedType* Edit();

    template <typename T>
    static ReflectedType* Create();

    static UnorderedMap<const RttiType*, ReflectedType*> typeToReflectedTypeMap;
    static UnorderedMap<String, ReflectedType*> permanentNameToReflectedTypeMap;
    static UnorderedMap<String, ReflectedType*> rttiNameToReflectedTypeMap;
};

} // namespace DAVA

#define __DAVA_ReflectedType__
#include "Reflection/Private/ReflectedType_impl.h"
