#pragma once

#include "Base/Set.h"

namespace DAVA
{
struct TrackedWatcher;
struct TrackedObject
{
    virtual ~TrackedObject();
    void DisconnectAll();

private:
    friend struct TrackedWatcher;
    Set<TrackedWatcher*> watchers;
};

} // namespace DAVA
