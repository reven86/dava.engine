#pragma once

#include "Base/Any.h"
#include "Base/BaseTypes.h"
#include "FileSystem/KeyedArchive.h"
#include "Platform/DateTime.h"

namespace DAVA
{
namespace Analytics
{
const char eventNameTag[] = "name";
const char eventCategoryTag[] = "category";
const char eventTimestamp[] = "timestamp";

struct EventRecord
{
    EventRecord(const String& name, const String& category)
    {
        fields[eventNameTag] = name;
        fields[eventCategoryTag] = category;
        fields[eventTimestamp] = DateTime::Now();
    }

    const Any* GetField(const String& field) const
    {
        auto iter = fields.find(field);
        return iter != fields.end() ? &iter->second : nullptr;
    }

    UnorderedMap<String, Any> fields;
};

struct IBackend
{
    virtual ~IBackend() = default;

    virtual void ConfigChanged(const KeyedArchive&) = 0;
    virtual void ProcessEvent(const EventRecord& event) = 0;
};

class Core
{
public:
    void Start();
    void Stop();
    bool IsStarted() const;

    void SetConfig(const KeyedArchive& newConfig);
    const KeyedArchive& GetConfig() const;

    void SetBackend(const String& name, std::shared_ptr<IBackend>& backend);
    bool PostEvent(const EventRecord& event) const;

private:
    RefPtr<KeyedArchive> config;
    UnorderedMap<String, std::shared_ptr<IBackend>> backends;
    bool isStarted = false;
};

} // namespace Analytics
} // namespace DAVA
