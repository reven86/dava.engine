#include "RenderCallbacks.h"
#include "Concurrency/LockGuard.h"
#include "Utils/Utils.h"

namespace DAVA
{
namespace
{
struct CallbackStruct
{
    Mutex mutex;
    Vector<Function<void()>> callbacks;

    void Add(Function<void()> cb)
    {
        LockGuard<Mutex> lock(mutex);
        callbacks.push_back(cb);
    }

    void Remove(Function<void()> cb)
    {
        LockGuard<Mutex> lock(mutex);
        callbacks.erase(std::remove_if(callbacks.begin(), callbacks.end(), [&cb](const Function<void()>& f)
                                       {
                                           return f.Target() == cb.Target();
                                       }),
                        callbacks.end());
    }

    void Execute()
    {
        locked = true;
        LockGuard<Mutex> lock(mutex);
        for (auto& cb : callbacks)
        {
            cb();
        }
        locked = false;
    }

private:
    bool locked = false;
};

struct SyncCallback
{
    rhi::HSyncObject syncObject;
    Function<void(rhi::HSyncObject)> callback;
};
Vector<SyncCallback> syncCallbacks;

Atomic<bool> restoreInProgress(false);
}

namespace RenderCallbacks
{
CallbackStruct restoreCallbacks;
CallbackStruct postRestoreCallbacks;

void RegisterResourceRestoreCallback(Function<void()> callback)
{
    DVASSERT(callback.IsTrivialTarget());
    restoreCallbacks.Add(callback);
}

void UnRegisterResourceRestoreCallback(Function<void()> callback)
{
    DVASSERT(callback.IsTrivialTarget());
    restoreCallbacks.Remove(callback);
}

void RegisterPostRestoreCallback(Function<void()> callback)
{
    DVASSERT(callback.IsTrivialTarget());
    postRestoreCallbacks.Add(callback);
}

void UnRegisterPostRestoreCallback(Function<void()> callback)
{
    DVASSERT(callback.IsTrivialTarget());
    postRestoreCallbacks.Remove(callback);
}

void ExecuteResourcesCallbacks()
{
    if (rhi::NeedRestoreResources())
    {
        restoreInProgress = true;
        restoreCallbacks.Execute();
    }
    else if (restoreInProgress)
    {
        postRestoreCallbacks.Execute();
        restoreInProgress = false;
    }
}

void ProcessFrame()
{
    ExecuteResourcesCallbacks();

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