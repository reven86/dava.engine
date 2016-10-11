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
    const KeyedArchive* eventsConfig = newConfig.GetArchive("events");
    if (eventsConfig == nullptr)
    {
        DVASSERT_MSG(false, "Illegal config: no events filtration configuration");
        return;
    }

    config.Set(new KeyedArchive(newConfig));

    // check on/off option
    if (config->IsKeyExists("started"))
    {
        if (config->GetBool("started"))
        {
            Start();
        }
        else
        {
            Stop();
        }
    }

    for (auto& backend : backends)
    {
        backend.second->ConfigChanged(*config);
    }
}

const KeyedArchive& Core::GetConfig() const
{
    return *config;
}

void Core::AddBackend(const String& name, const std::shared_ptr<IBackend>& backend)
{
    backends[name] = backend;
}

bool CheckEventPass(const KeyedArchive& config, const EventRecord& event)
{
    // check all-passing option
    if (config.GetBool("all"))
    {
        return true;
    }

    String eventName;
    const Any* nameField = event.GetField(eventNameTag);
    if (nameField->CanCast<String>())
    {
        eventName = nameField->Cast<String>();
    }
    else if (nameField->CanGet<const char*>())
    {
        eventName = nameField->Get<const char*>();
    }

    const KeyedArchive::UnderlyingMap& map = config.GetArchieveData();
    for (const auto& entry : map)
    {
        if (entry.first == eventName)
        {
            return true;
        }
    }

    return false;
}

bool Core::PostEvent(const EventRecord& event) const
{
    if (!IsStarted() || backends.empty() || !config->IsKeyExists("events"))
    {
        return false;
    }

    // common filter
    bool isEventPasses = CheckEventPass(*config->GetArchive("events"), event);
    if (!isEventPasses)
    {
        return false;
    }

    // per-backend filter
    const KeyedArchive* backendsConfig = config->GetArchive("backends");
    for (const auto& backend : backends)
    {
        bool needProcessEvent = true;

        if (backendsConfig)
        {
            const KeyedArchive* backendConfig = backendsConfig->GetArchive(backend.first);

            const KeyedArchive* backendEvents;
            backendEvents = backendConfig ? backendConfig->GetArchive("events") : nullptr;

            if (backendEvents)
            {
                needProcessEvent = CheckEventPass(*backendEvents, event);
            }
        }

        if (needProcessEvent)
        {
            backend.second->ProcessEvent(event);
        }
    }

    return true;
}

} // namespace Analytics
} // namespace DAVA
