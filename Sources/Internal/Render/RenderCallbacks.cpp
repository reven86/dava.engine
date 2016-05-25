#include "RenderCallbacks.h"
#include "Concurrency/LockGuard.h"
#include "Utils/Utils.h"

namespace DAVA
{
namespace
{
enum class CallbackOperation : uint32
{
    Add,
    Execute,
    Remove
};

using Callback = Function<void()>;

struct StorageComparator
{
    bool operator()(const Fn11::Closure::Storage& l, const Fn11::Closure::Storage& r)
    {
        uint64 l0 = reinterpret_cast<uintptr_t>(l[0]);
        uint64 l1 = reinterpret_cast<uintptr_t>(l[1]);
        uint64 r0 = reinterpret_cast<uintptr_t>(r[0]);
        uint64 r1 = reinterpret_cast<uintptr_t>(r[1]);
        return (l0 ^ l1) < (r0 ^ r1);
    }
};

struct CallbackStruct
{
    using OpPair = std::pair<CallbackOperation, Function<void()>>;
    using CbMap = Map<Fn11::Closure::Storage, OpPair, StorageComparator>;

    Mutex pendingMutex;
    Mutex activeMutex;
    CbMap pending;
    CbMap active;

    void Add(Callback cb)
    {
        if (locked)
        {
            LockGuard<Mutex> guard(pendingMutex);
            pending[cb.Target()] = std::make_pair(CallbackOperation::Add, cb);
        }
        else
        {
            LockGuard<Mutex> guard(activeMutex);
            active[cb.Target()] = std::make_pair(CallbackOperation::Execute, cb);
        }
    }

    void Remove(Callback cb)
    {
        LockGuard<Mutex> guard(activeMutex);
        auto i = active.find(cb.Target());
        if (i != active.end())
        {
            i->second.first = CallbackOperation::Remove;
        }

        LockGuard<Mutex> pendingGuard(pendingMutex);
        pending[cb.Target()] = std::make_pair(CallbackOperation::Remove, cb);
    }

    void Merge()
    {
        LockGuard<Mutex> guard(activeMutex);
        LockGuard<Mutex> pendingGuard(pendingMutex);
        for (auto& e : pending)
        {
            if (e.second.first == CallbackOperation::Add)
            {
                active[e.first] = std::make_pair(CallbackOperation::Execute, e.second.second);
            }
            else if (e.second.first == CallbackOperation::Remove)
            {
                active.erase(e.first);
            }
        }
        pending.clear();
    }

    void Execute()
    {
        Merge();

        locked = true;

        LockGuard<Mutex> guard(activeMutex);
        for (auto& cb : active)
        {
            if (cb.second.first == CallbackOperation::Execute)
            {
                cb.second.second();
            }
        }

        locked = false;
    }

private:
    bool locked = false;
};

CallbackStruct restoreCallbacks;
CallbackStruct postRestoreCallbacks;

struct SyncCallback
{
    rhi::HSyncObject syncObject;
    Function<void(rhi::HSyncObject)> callback;
};
Vector<SyncCallback> syncCallbacks;

Atomic<bool> restoreInProgress = false;
}

namespace RenderCallbacks
{
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

void ProcessFrame()
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