#pragma once

#include <Reflection/Reflection.h>
#include <Reflection/ReflectedType.h>
#include <Reflection/ReflectedStructure.h>
#include <Functional/Function.h>

namespace DAVA
{
namespace TArc
{
void ForEachField(const Reflection& r, const Function<void(Reflection::Field&& field)>& fn);

const ReflectedType* GetReflectedType(const Reflection& r);
const ReflectedType* GetReflectedType(const Any& value);

template <typename T>
const T* GetTypeMeta(const Any& value)
{
    const ReflectedType* type = GetReflectedType(value);
    if (type == nullptr)
    {
        return nullptr;
    }

    const ReflectedStructure* structure = type->GetStructure();
    if (structure == nullptr)
    {
        return nullptr;
    }

    if (structure->meta == nullptr)
    {
        return nullptr;
    }

    return structure->meta->GetMeta<T>();
}
} // namespace TArc
} // namespace DAVA