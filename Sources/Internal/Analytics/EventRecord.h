#pragma once

#include "Base/Any.h"
#include "Base/BaseTypes.h"
#include "Platform/DateTime.h"

namespace DAVA
{
namespace Analytics
{
const char eventNameTag[] = "name";
const char eventTimestampTag[] = "timestamp";

struct EventRecord
{
    EventRecord(const String& name)
    {
        fields[eventNameTag] = name;
        fields[eventTimestampTag] = DateTime::Now();
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
