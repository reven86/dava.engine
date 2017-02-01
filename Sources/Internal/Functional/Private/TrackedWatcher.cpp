#include "Functional/TrackedObject.h"
#include "Functional/Private/TrackedWatcher.h"

namespace DAVA
{
void TrackedWatcher::Watch(TrackedObject* object)
{
    object->watchers.insert(this);
}

void TrackedWatcher::Unwatch(TrackedObject* object)
{
    object->watchers.erase(this);
}
} //namespace DAVA
