#include "Analytics/Analytics.h"

namespace DAVA
{
namespace Analytics
{
void Core::Start()
{
    isStarted = true;
}

void Core::Stop()
{
    isStarted = false;
}

bool Core::IsStarted() const
{
    return isStarted;
}

void Core::SetConfig(const KeyedArchive& newConfig)
{
    config.Set(new KeyedArchive(newConfig));

    for (auto& backend : backends)
    {
        backend.second->ConfigChanged(*config);
    }
}

const KeyedArchive& Core::GetConfig() const
{
    return *config;
}

void Core::SetBackend(const String& name, std::shared_ptr<IBackend>& backend)
{
    backends[name] = backend;
}

bool Core::PostEvent(const EventRecord& event) const
{
    if (!IsStarted())
    {
        return false;
    }

    for (const auto& backend : backends)
    {
        backend.second->ProcessEvent(event);
    }
    return true;
}

} // namespace Analytics
} // namespace DAVA
