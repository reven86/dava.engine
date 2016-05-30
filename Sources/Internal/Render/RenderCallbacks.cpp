#include "RenderCallbacks.h"
#include "Concurrency/LockGuard.h"
#include "Utils/Utils.h"

namespace DAVA
{
namespace
{
struct SyncCallback
{
    rhi::HSyncObject syncObject;
    Function<void(rhi::HSyncObject)> callback;
};
Vector<SyncCallback> syncCallbacks;

Vector<Function<void()>> restoreCallbacks;
Mutex restoreMutex;

Vector<Function<void()>> postRestoreCallbacks;
Mutex postRestoreMutex;

Atomic<bool> restoreInProgress(false);
}

namespace RenderCallbacks
{
void RegisterResourceRestoreCallback(Function<void()> callback)
{
    DVASSERT(callback.IsTrivialTarget());
    LockGuard<Mutex> lock(restoreMutex);
    if (restoreCallbacks.empty())
    {
        restoreCallbacks.reserve(256);
    }
    restoreCallbacks.push_back(callback);
}

void UnRegisterResourceRestoreCallback(Function<void()> callback)
{
    DVASSERT(callback.IsTrivialTarget());
    LockGuard<Mutex> lock(restoreMutex);
    restoreCallbacks.erase(std::remove_if(restoreCallbacks.begin(), restoreCallbacks.end(), [&callback](const Function<void()>& f)
                                          {
                                              return f.Target() == callback.Target();
                                          }),
                           restoreCallbacks.end());
}

void RegisterPostRestoreCallback(Function<void()> callback)
{
    DVASSERT(callback.IsTrivialTarget());
    LockGuard<Mutex> lock(postRestoreMutex);
    if (postRestoreCallbacks.empty())
    {
        postRestoreCallbacks.reserve(256);
    }
    postRestoreCallbacks.push_back(callback);
}

void UnRegisterPostRestoreCallback(Function<void()> callback)
{
    DVASSERT(callback.IsTrivialTarget());
    LockGuard<Mutex> lock(postRestoreMutex);
    postRestoreCallbacks.erase(std::remove_if(postRestoreCallbacks.begin(), postRestoreCallbacks.end(),
                                              [&callback](const Function<void()>& f)
                                              {
                                                  return f.Target() == callback.Target();
                                              }),
                               postRestoreCallbacks.end());
}

void ExecuteResourcesCallbacks()
{
    if (rhi::NeedRestoreResources())
    {
        restoreInProgress = true;
        LockGuard<Mutex> lock(restoreMutex);
        for (auto& cb : restoreCallbacks)
        {
            cb();
        }
    }
    else if (restoreInProgress)
    {
        LockGuard<Mutex> lock(postRestoreMutex);
        for (auto& cb : postRestoreCallbacks)
        {
            cb();
        }
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