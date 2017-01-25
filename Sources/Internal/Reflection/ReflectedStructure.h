#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
class Type;
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
        std::unique_ptr<ValueWrapper> valueWrapper;
        std::unique_ptr<ReflectedMeta> meta;

        const ReflectedType* reflectedType;
    };

    struct Method
    {
        String name;
        AnyFn method;
        std::unique_ptr<ReflectedMeta> meta;
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

    Vector<std::unique_ptr<AnyFn>> ctors;
    std::unique_ptr<AnyFn> dtor;
};
} // namespace DAVA
