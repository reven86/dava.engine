#include "RenderCallbacks.h"
#include "Concurrency/LockGuard.h"
#include "Utils/Utils.h"

namespace DAVA
{
namespace
{
Mutex callbackListMutex;
Vector<Function<void()>> resourceRestoreCallbacks;
Vector<Function<void()>> postRestoreCallbacks;

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
    LockGuard<Mutex> guard(callbackListMutex);
    resourceRestoreCallbacks.push_back(callback);
}
void UnRegisterResourceRestoreCallback(Function<void()> callback)
{
    DVASSERT(callback.IsTrivialTarget());
    LockGuard<Mutex> guard(callbackListMutex);
    for (size_t i = 0, sz = resourceRestoreCallbacks.size(); i < sz; ++i)
    {
        if (resourceRestoreCallbacks[i].Target() == callback.Target())
        {
            RemoveExchangingWithLast(resourceRestoreCallbacks, i);
            return;
        }
    }
    DVASSERT_MSG(false, "trying to unregister callback that was not perviously registered");
}

void RegisterPostRestoreCallback(Function<void()> callback)
{
    DVASSERT(callback.IsTrivialTarget());
    LockGuard<Mutex> guard(callbackListMutex);
    postRestoreCallbacks.push_back(callback);
}
void UnRegisterPostRestoreCallback(Function<void()> callback)
{
    DVASSERT(callback.IsTrivialTarget());
    LockGuard<Mutex> guard(callbackListMutex);
    for (size_t i = 0, sz = postRestoreCallbacks.size(); i < sz; ++i)
    {
        if (postRestoreCallbacks[i].Target() == callback.Target())
        {
            RemoveExchangingWithLast(postRestoreCallbacks, i);
            return;
        }
    }
    DVASSERT_MSG(false, "trying to unregister callback that was not perviously registered");
}

void ProcessFrame()
{
    if (rhi::NeedRestoreResources())
    {
        isInRestore = true;
        LockGuard<Mutex> guard(callbackListMutex);
        for (auto& callback : resourceRestoreCallbacks)
        {
            callback();
        }
        Logger::Debug("Resources still need restore: ");
        rhi::NeedRestoreResources();
    }
    else
    {
        if (isInRestore)
        {
            isInRestore = false;
            LockGuard<Mutex> guard(callbackListMutex);
            for (auto& callback : postRestoreCallbacks)
            {
                callback();
            }
        }
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