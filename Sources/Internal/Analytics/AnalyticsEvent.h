#pragma once

#include "Base/Any.h"
#include "Base/BaseTypes.h"
#include "Platform/DateTime.h"

namespace DAVA
{
namespace Analytics
{
/**
AnalyticsEvent represents information about analytics event
*/
struct AnalyticsEvent
{
    AnalyticsEvent(const String& eventName)
        : name(eventName)
        , timestamp(DateTime::Now())
    {
    }

    const Any* GetField(const String& field) const
    {
        auto iter = fields.find(field);
        return iter != fields.end() ? &iter->second : nullptr;
    }

    String name;
    DateTime timestamp;
    UnorderedMap<String, Any> fields;
};

} // namespace Analytics
} // namespace DAVA
