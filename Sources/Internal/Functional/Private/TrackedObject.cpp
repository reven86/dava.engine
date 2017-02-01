#include "Functional/TrackedObject.h"
#include "Functional/Private/TrackedWatcher.h"

namespace DAVA
{
TrackedObject::~TrackedObject()
{
    DisconnectAll();
}

void TrackedObject::DisconnectAll()
{
    auto it = watchers.begin();
    auto end = watchers.end();
    while (it != end)
    {
        TrackedWatcher* watcher = *it;
        it++;
        watcher->OnTrackedObjectDisconnect(this);
    }

    watchers.clear();
}
} // namespace DAVA
