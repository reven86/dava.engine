#pragma once

#include <memory>
#include "Base/RttiType.h"
#include "Reflection/ReflectedMeta.h"
#include "Reflection/Wrappers.h"

namespace DAVA
{
class ReflectedType;
class ReflectedStructure final
{
public:
    struct Field
    {
        String name;
        std::unique_ptr<ReflectedMeta> meta;
        std::unique_ptr<ValueWrapper> fieldWrapper;

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
    std::unique_ptr<ReflectedMeta> meta;
};
} // namespace DAVA
