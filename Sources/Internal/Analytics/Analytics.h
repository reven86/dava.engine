#pragma once

#include "Analytics/AnalyticsEvent.h"
#include "Base/BaseTypes.h"
#include "FileSystem/KeyedArchive.h"

namespace DAVA
{
class KeyedArchive;

namespace Analytics
{
/**
Analytics backend interface
Backend should handle event and store or sent to analytics service like Google Analytics

Example:
struct PrintBackend : public IBackend
{
    void ConfigChanged(const KeyedArchive& config) override {}
    void ProcessEvent(const AnalyticsEvent& event) override 
    {
        printf("Event name: %s", event.name.c_str());
    }
}
*/
struct IBackend
{
    virtual ~IBackend() = default;

    /** Handle changing of analytics system config */
    virtual void ConfigChanged(const KeyedArchive& config) = 0;

    /** Handle analytics event */
    virtual void ProcessEvent(const AnalyticsEvent& event) = 0;
};

/**
Analytics core class
Entry point for all analytics events. Core filters it and sends to backends
For events processing Core should be configured and set to started state; backends should be added
*/
class Core
{
public:
    /** Start event processing (set to started state) */
    void Start();

    /** Stop event processing (set to not-started state) */
    void Stop();

    /** Return if event processing started */
    bool IsStarted() const;

    /**
    Set config to analytics system
    Config should contains:
    - options for configuration of the system and backends
        Has key-value form, so access to it looks like config->GetString("option")
        Option "started" has bool type and needed to set Core to started or not-started state
    - common events filtration config
        Required config, has key "events" and subtree with key-value options form
        Subtree contains list of allowed event names
        Bool option "all" used to allow all events
    - per-backend events filtration config
        Optional config, has key "backend_events" and contain list of subtrees for events filtration
        such as mentioned before

    Example of config (creation by code):
    KeyedArchive* config = new KeyedArchive;
    config->SetBool("started", true);        // Set started state
    config->SetString("google_id", "ID#777");
    KeyedArchive* events = new KeyedArchive; // Create common events filtration config
    events->SetBool("all", true);            // Allow all events
    config->SetArchive("events", events);
    
    KeyedArchive* backendEvents = new KeyedArchive;

    // Create "google" backend config and disallow all events
    KeyedArchive* googleBackend = new KeyedArchive;
    googleBackend->SetBool("all", false);
    backendEvents->SetArchive("google", googleBackend);

    // Create "printf" backend config and allow only hangar events
    KeyedArchive* printfBackend = new KeyedArchive;
    printf->SetString("Hangar", "");
    backendEvents->SetArchive("printf", printfBackend);

    config->SetArchive("backend_events", backendEvents);
    */
    void SetConfig(const KeyedArchive* newConfig);

    /** Return config */
    const KeyedArchive* GetConfig() const;

    /** Add backend */
    void AddBackend(const String& name, std::unique_ptr<IBackend> backend);

    /** 
    Post event for filtration and processing
    Method filters events by using common and per-backend filtration config and then sends them to backends
    If Core has not-started state, is not configured or has no backends, any event will not pass
    If event don't satisfy common filtration config, it will not pass
    If per-backend filtration config doesn't exist, only common filtration config 
    will be applied to event and event will be sent to all backends
    Otherwise, event will be sent to only those backends whose config it satisfies
    Return true if event was sent to at least one backend
    */
    bool PostEvent(const AnalyticsEvent& event) const;

private:
    RefPtr<KeyedArchive> config;
    UnorderedMap<String, std::unique_ptr<IBackend>> backends;
    bool isStarted = false;
};

} // namespace Analytics
} // namespace DAVA
