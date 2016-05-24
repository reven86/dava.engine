#include "RenderCallbacks.h"
#include "Concurrency/LockGuard.h"
#include "Utils/Utils.h"

namespace DAVA
{
namespace
{
enum class CallbackOperation : uint32
{
    Execute,
    Remove
};

Mutex pendingListMutex;
Vector<Function<void()>> pendingRestoreCallbacks;

Mutex callbackListMutex;
Vector<std::pair<CallbackOperation, Function<void()>>> resourceRestoreCallbacks;
Vector<std::pair<CallbackOperation, Function<void()>>> postRestoreCallbacks;

struct SyncCallback
{
    rhi::HSyncObject syncObject;
    Function<void(rhi::HSyncObject)> callback;
};
Vector<SyncCallback> syncCallbacks;

bool isInRestore = false;
}

namespace RenderCallbacks
{
void RegisterResourceRestoreCallback(Function<void()> callback)
{
    DVASSERT(callback.IsTrivialTarget());
    if (isInRestore)
    {
        LockGuard<Mutex> guard(pendingListMutex);
        pendingRestoreCallbacks.push_back(callback);
    }
    else
    {
        LockGuard<Mutex> guard(callbackListMutex);
        resourceRestoreCallbacks.emplace_back(CallbackOperation::Execute, callback);
    }
}

void UnRegisterResourceRestoreCallback(Function<void()> callback)
{
    DVASSERT(callback.IsTrivialTarget());

    bool found = false;
    LockGuard<Mutex> guard(callbackListMutex);
    for (auto& cb : resourceRestoreCallbacks)
    {
        if (cb.second.Target() == callback.Target())
        {
            cb.first = CallbackOperation::Remove;
            found = true;
        }
    }

    LockGuard<Mutex> pendingGuard(pendingListMutex);
    for (auto i = pendingRestoreCallbacks.begin(); i != pendingRestoreCallbacks.end();)
    {
        if (i->Target() == callback.Target())
        {
            found = true;
            i = pendingRestoreCallbacks.erase(i);
        }
        else
        {
            ++i;
        }
    }

    DVASSERT(found);
}

void RegisterPostRestoreCallback(Function<void()> callback)
{
    DVASSERT(callback.IsTrivialTarget());
    LockGuard<Mutex> guard(callbackListMutex);
    postRestoreCallbacks.emplace_back(CallbackOperation::Execute, callback);
}

void UnRegisterPostRestoreCallback(Function<void()> callback)
{
    DVASSERT(callback.IsTrivialTarget());
    LockGuard<Mutex> guard(callbackListMutex);
    for (size_t i = 0, sz = postRestoreCallbacks.size(); i < sz; ++i)
    {
        auto& cb = postRestoreCallbacks[i];
        if (cb.second.Target() == callback.Target())
        {
            cb.first = CallbackOperation::Remove;
            return;
        }
    }
    DVASSERT_MSG(false, "trying to unregister callback that was not perviously registered");
}

void ProcessFrame()
{
    if (!pendingRestoreCallbacks.empty())
    {
        LockGuard<Mutex> pendingGuard(pendingListMutex);
        LockGuard<Mutex> cbGuard(callbackListMutex);
        for (const auto& i : pendingRestoreCallbacks)
        {
            resourceRestoreCallbacks.emplace_back(CallbackOperation::Execute, i);
        }
        pendingRestoreCallbacks.clear();
    }

    if (rhi::NeedRestoreResources())
    {
        isInRestore = true;
        for (auto i = resourceRestoreCallbacks.begin(); i != resourceRestoreCallbacks.end();)
        {
            if (i->first == CallbackOperation::Execute)
            {
                i->second();
                ++i;
            }
            else
            {
                i = resourceRestoreCallbacks.erase(i);
            }
        }
    }
    else if (isInRestore)
    {
        for (auto i = postRestoreCallbacks.begin(); i != postRestoreCallbacks.end();)
        {
            if (i->first == CallbackOperation::Execute)
            {
                i->second();
                ++i;
            }
            else
            {
                i = postRestoreCallbacks.erase(i);
            }
        }
        isInRestore = false;
    }

    for (size_t i = 0, sz = syncCallbacks.size(); i < sz;)
    {
        if (rhi::SyncObjectSignaled(syncCallbacks[i].syncObject))
        {
            syncCallbacks[i].callback(syncCallbacks[i].syncObject);
            RemoveExchangingWithLast(syncCallbacks, i);
            --sz;
        }
        else
        {
            ++i;
        }
    }
}

void RegisterSyncCallback(rhi::HSyncObject syncObject, Function<void(rhi::HSyncObject)> callback)
{
    syncCallbacks.push_back({ syncObject, callback });
}

void UnRegisterSyncCallback(Function<void(rhi::HSyncObject)> callback)
{
    for (size_t i = 0, sz = syncCallbacks.size(); i < sz;)
    {
        if (syncCallbacks[i].callback.Target() == callback.Target())
        {
            RemoveExchangingWithLast(syncCallbacks, i);
            --sz;
        }
        else
        {
            ++i;
        }
    }
}
}
}