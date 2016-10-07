#pragma once

#include "Analytics/EventRecord.h"
#include "Base/BaseTypes.h"
#include "FileSystem/KeyedArchive.h"

namespace DAVA
{
namespace Analytics
{
struct IBackend
{
    virtual ~IBackend() = default;

    virtual void ConfigChanged(const KeyedArchive& config) = 0;
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

    void AddBackend(const String& name, const std::shared_ptr<IBackend>& backend);
    bool PostEvent(const EventRecord& event) const;

private:
    RefPtr<KeyedArchive> config;
    UnorderedMap<String, std::shared_ptr<IBackend>> backends;
    bool isStarted = false;
};

} // namespace Analytics
} // namespace DAVA
