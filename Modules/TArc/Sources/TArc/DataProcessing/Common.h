#pragma once

#include "Base/FastName.h"
#include "Reflection/ReflectedType.h"

namespace DAVA
{
namespace TArc
{
struct FieldDescriptor
{
    const ReflectedType* type = nullptr;
    FastName fieldName;

    FieldDescriptor() = default;

    FieldDescriptor(const ReflectedType* type_, const FastName& fieldName_)
        : type(type_)
        , fieldName(fieldName_)
    {
    }
};

} // namespace TArc
} // namespace DAVA