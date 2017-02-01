#pragma once

namespace DAVA
{
struct TrackedObject;
struct TrackedWatcher
{
public:
    void Watch(TrackedObject* object);
    void Unwatch(TrackedObject* object);

    virtual void OnTrackedObjectDisconnect(TrackedObject*) = 0;
};
} // namespace DAVA
