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
};
} // namespace TArc
} // namespace DAVA