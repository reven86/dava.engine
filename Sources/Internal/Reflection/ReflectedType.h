#pragma once

#include <memory>
#include "Base/Type.h"
#include "Base/AnyFn.h"
#include "Reflection/Wrappers.h"

namespace DAVA
{
class ReflectedMeta;
class ReflectedObject;

struct ReflectedPropertie
{
    const ValueWrapper* valueWrapper;
    const ReflectedMeta* meta;
    const ReflectedType* rtype;
};

class ReflectedEnum final
{
public:
    std::unique_ptr<EnumWrapper> enumWrapper;
};

class ReflectedMethod final
{
public:
    std::unique_ptr<MethodWrapper> methodWrapper;
    std::unique_ptr<ReflectedMeta> meta;
};

class ReflectedStructure final
{
public:
    Vector<std::unique_ptr<CtorWrapper>> ctors;
    Vector<std::unique_ptr<DtorWrapper>> dtors;
    Vector<std::unique_ptr<Field>> fields;
    Vector<std::unique_ptr<Method>> methods;
    Vector<std::unique_ptr<Enum>> enums;
};

class ReflectedType final
{
public:
    String name;
    std::unique_ptr<ReflectedStructure> structure;
    std::unique_ptr<StructureWrapper> structureWrapper;
    std::unique_ptr<ReflectedMeta> meta;
};

} // namespace DAVA
