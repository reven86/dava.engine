#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
namespace Analytics
{
{
    const char eventNameTag[] = "name";
    const char eventCategoryTag[] = "category";
    const char eventTimestamp[] = "timestamp";

    struct EventRecord
    {
        EventRecord(const String& name, const String& category);

        template <typename T>
        const T& GetField(const String& field) const;

        template <typename T>
        T CastField(const String& field) const;

        bool HasField(const String& field) const;

        UnorderedMap<String, Any> fields;
    };

    class Core
    {
    public:
        void SetConfig(KeyedArchive& newConfig);
        const KeyedArchive& GetConfig() const;

        Signal<const EventRecord&>& GetEventSignal(const String& backend);
        Signal<const KeyedArchive&> onConfigChanged;

        bool PostEvent(const EventRecord& event) const;

    private:
        RefPtr<KeyedArchive> config;
        UnorderedMap<String, Signal<const EventRecord&>> backendSignals;
    };

} // namespace Analytics
} // namespace DAVA
