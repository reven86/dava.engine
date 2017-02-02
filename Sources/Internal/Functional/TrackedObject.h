#pragma once

#include "Base/Set.h"

namespace DAVA
{
struct SignalBase;
struct TrackedObject
{
    virtual ~TrackedObject();
    void DisconnectAll();

private:
    friend struct SignalBase;
    Set<SignalBase*> watchers;
};

} // namespace DAVA
