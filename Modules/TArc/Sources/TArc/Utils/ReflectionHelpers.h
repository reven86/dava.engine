#pragma once

#include <Reflection/Reflection.h>
#include <Reflection/ReflectedType.h>
#include <Reflection/ReflectedMeta.h>
#include <Reflection/ReflectedStructure.h>
#include <Reflection/ReflectedTypeDB.h>
#include <Functional/Function.h>

namespace DAVA
{
namespace TArc
{
void ForEachField(const Reflection& r, const Function<void(Reflection::Field&& field)>& fn);

const ReflectedType* GetValueReflectedType(const Reflection& r);
const ReflectedType* GetValueReflectedType(const Any& value);

template <typename T>
const T* GetTypeMeta(const Any& value)
{
    const ReflectedType* type = GetValueReflectedType(value);
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

template <typename TMeta, typename TIndex>
void EmplaceTypeMeta(ReflectedType* type, Meta<TMeta, TIndex>&& meta)
{
    ReflectedStructure* structure = type->GetStructure();
    DVASSERT(structure != nullptr);

    if (structure->meta == nullptr)
    {
        structure->meta.reset(new ReflectedMeta());
    }

    structure->meta->Emplace(std::move(meta));
}

template <typename T, typename TMeta, typename TIndex>
void EmplaceTypeMeta(Meta<TMeta, TIndex>&& meta)
{
    ReflectedType* type = const_cast<ReflectedType*>(ReflectedTypeDB::Get<T>());
    DVASSERT(type != nullptr);

    EmplaceTypeMeta(type, std::move(meta));
}

template <typename T, typename TMeta, typename TIndex>
void EmplaceFieldMeta(const String& fieldName, Meta<TMeta, TIndex>&& meta)
{
    ReflectedType* type = const_cast<ReflectedType*>(ReflectedTypeDB::Get<T>());
    DVASSERT(type != nullptr);

    ReflectedStructure* structure = type->GetStructure();
    DVASSERT(structure != nullptr);

    for (std::unique_ptr<ReflectedStructure::Field>& field : structure->fields)
    {
        if (field->name == fieldName)
        {
            if (field->meta == nullptr)
            {
                field->meta.reset(new ReflectedMeta());
            }

            field->meta->Emplace(std::move(meta));
            break;
        }
    }
}
} // namespace TArc
} // namespace DAVA