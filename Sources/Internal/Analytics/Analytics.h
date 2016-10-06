#pragma once

#include "Base/BaseTypes.h"
#include "FileSystem/KeyedArchive.h"

namespace DAVA
{
namespace Analytics
{
const char eventNameTag[] = "name";
const char eventCategoryTag[] = "category";
const char eventTimestamp[] = "timestamp";

struct EventRecord
{
    EventRecord(const String& name, const String& category);
    const Any* GetField(const String& field) const;
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
    void SetConfig(KeyedArchive& newConfig);
    const KeyedArchive& GetConfig() const;

    void SetBackend(const String& name, std::shared_ptr<IBackend>& backend);
    bool PostEvent(const EventRecord& event) const;

private:
    RefPtr<KeyedArchive> config;
    UnorderedMap<String, std::shared_ptr<IBackend>> backends;
};

} // namespace Analytics
} // namespace DAVA
