#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
class RtType;
class ReflectedMeta;
class ReflectedType;
class ValueWrapper;
class MethodWrapper;
class EnumWrapper;
class CtorWrapper;
class DtorWrapper;

class ReflectedStructure final
{
public:
    struct Field
    {
        String name;
        std::unique_ptr<ReflectedMeta> meta;
        std::unique_ptr<ValueWrapper> valueWrapper;

        const ReflectedType* reflectedType;
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

    std::unique_ptr<ReflectedMeta> meta;

    Vector<std::unique_ptr<Field>> fields;
    Vector<std::unique_ptr<Method>> methods;
    Vector<std::unique_ptr<Enum>> enums;

    Vector<std::unique_ptr<CtorWrapper>> ctors;
    std::unique_ptr<DtorWrapper> dtor;
};
} // namespace DAVA
