#pragma once

#include "Base/Any.h"
#include "Base/BaseTypes.h"

namespace DAVA
{
namespace Analytics
{
const char eventNameTag[] = "name";
const char eventCategoryTag[] = "category";

struct EventRecord
{
    EventRecord(const String& category, const String& name)
    {
        fields[eventCategoryTag] = category;
        fields[eventNameTag] = name;
    }

    const Any* GetField(const String& field) const
    {
        auto iter = fields.find(field);
        return iter != fields.end() ? &iter->second : nullptr;
    }

    UnorderedMap<String, Any> fields;
};

} // namespace Analytics
} // namespace DAVA
