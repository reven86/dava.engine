#ifndef __DAVAENGINE_SERVICEREGISTRAR_H__
#define __DAVAENGINE_SERVICEREGISTRAR_H__

#include <Base/BaseTypes.h>
#include <Functional/Function.h>
#include "Network/NetCallbacksHolder.h"
#include "Logger/Logger.h"
#include "Concurrency/Thread.h"
#include <chrono>

namespace DAVA
{
namespace Net
{
struct IChannelListener;

using ServiceCreator = Function<IChannelListener*(uint32 serviceId, void* context)>;
using ServiceDeleter = Function<void(IChannelListener* obj, void* context)>;

class ServiceCreatorExecutor
{
public:
    explicit ServiceCreatorExecutor(ServiceCreator fn, uint32 serviceId, void* context)
        : targetFn(fn)
        , serviceId(serviceId)
        , context(context)
    {
    }

    void ServiceCreatorCall()
    {
        auto duration = std::chrono::system_clock::now().time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        Logger::Debug("thread %u (%d): %s", Thread::GetCurrentId(), millis, __FUNCTION__);
        targetFnResult = targetFn(serviceId, context);
        cvDone.NotifyAll();
    }

    uint32 serviceId = 0;
    void* context = nullptr;
    ServiceCreator targetFn;
    IChannelListener* targetFnResult = nullptr;
    Mutex cvMutex;
    ConditionVariable cvDone;
};

class ServiceCreatorAsync
{
public:
    explicit ServiceCreatorAsync(ServiceCreator targetFn, NetCallbacksHolder* holder)
        : targetFn(targetFn)
        , holder(holder)
    {
    }

    IChannelListener* ServiceCreatorCall(uint32 serviceId, void* context)
    {
        auto duration = std::chrono::system_clock::now().time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        Logger::Debug("thread %u (%d): %s", Thread::GetCurrentId(), millis, __FUNCTION__);

        if (holder->GetMode() == NetCallbacksHolder::ExecuteImmediately)
        {
            return targetFn(serviceId, context);
        }
        else
        {
            std::shared_ptr<ServiceCreatorExecutor> executor(new ServiceCreatorExecutor(targetFn, serviceId, context));
            std::weak_ptr<ServiceCreatorExecutor> executorWeak = executor;
            auto fn = MakeFunction(executorWeak, &ServiceCreatorExecutor::ServiceCreatorCall);
            holder->AddCallback(fn);
            executor->cvDone.Wait(executor->cvMutex);
            return executor->targetFnResult;
        }
    }

    ServiceCreator targetFn;
    NetCallbacksHolder* holder = nullptr;
};

class ServiceDeleterExecutor
{
public:
    explicit ServiceDeleterExecutor(ServiceDeleter fn, IChannelListener* obj, void* context)
        : targetFn(fn)
        , obj(obj)
        , context(context)
    {
    }

    void ServiceDeleterCall()
    {
        auto duration = std::chrono::system_clock::now().time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        Logger::Debug("thread %u (%d): %s", Thread::GetCurrentId(), millis, __FUNCTION__);
        targetFn(obj, context);
        cvDone.NotifyAll();
    }

    IChannelListener* obj = nullptr;
    void* context = nullptr;
    ServiceDeleter targetFn;
    Mutex cvMutex;
    ConditionVariable cvDone;
};

class ServiceDeleterAsync
{
public:
    explicit ServiceDeleterAsync(ServiceDeleter targetFn, NetCallbacksHolder* holder)
        : targetFn(targetFn)
        , holder(holder)
    {
    }

    void ServiceDeleterCall(IChannelListener* obj, void* context)
    {
        auto duration = std::chrono::system_clock::now().time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        Logger::Debug("thread %u (%d): %s", Thread::GetCurrentId(), millis, __FUNCTION__);

        if (holder->GetMode() == NetCallbacksHolder::ExecuteImmediately)
        {
            targetFn(obj, context);
        }
        else
        {
            std::shared_ptr<ServiceDeleterExecutor> executor(new ServiceDeleterExecutor(targetFn, obj, context));
            auto fn = MakeFunction(executor, &ServiceDeleterExecutor::ServiceDeleterCall);
            holder->AddCallback(fn);
            executor->cvDone.Wait(executor->cvMutex);
        }
    }

    ServiceDeleter targetFn;
    NetCallbacksHolder* holder = nullptr;
};

class ServiceRegistrar
{
private:
    struct Entry
    {
        static const size_t MAX_NAME_LENGTH = 32;

        Entry(uint32 id, const char8* serviceName, ServiceCreator creatorFunc, ServiceDeleter deleterFunc);

        uint32 serviceId;
        char8 name[MAX_NAME_LENGTH];
        ServiceCreator creator;
        ServiceDeleter deleter;
    };

    friend bool operator==(const Entry& entry, uint32 serviceId);

public:
    bool Register(uint32 serviceId, ServiceCreator creator, ServiceDeleter deleter, const char8* name = NULL);
    bool UnRegister(uint32 serviceId);
    void UnregisterAll();
    bool IsRegistered(uint32 serviceId) const;

    IChannelListener* Create(uint32 serviceId, void* context) const;
    bool Delete(uint32 serviceId, IChannelListener* obj, void* context) const;

    const char8* Name(uint32 serviceId) const;

private:
    const Entry* FindEntry(uint32 serviceId) const;

private:
    Vector<Entry> registrar;
};

//////////////////////////////////////////////////////////////////////////
inline ServiceRegistrar::Entry::Entry(uint32 id, const char8* serviceName, ServiceCreator creatorFunc, ServiceDeleter deleterFunc)
    : serviceId(id)
    , creator(creatorFunc)
    , deleter(deleterFunc)
{
#if defined(__DAVAENGINE_WINDOWS__)
    strncpy_s(name, serviceName, _TRUNCATE);
#else
    strncpy(name, serviceName, MAX_NAME_LENGTH);
    name[MAX_NAME_LENGTH - 1] = '\0';
#endif
}

inline bool ServiceRegistrar::IsRegistered(uint32 serviceId) const
{
    return std::find(registrar.begin(), registrar.end(), serviceId) != registrar.end();
}

inline bool operator==(const ServiceRegistrar::Entry& entry, uint32 serviceId)
{
    return entry.serviceId == serviceId;
}

} // namespace Net
} // namespace DAVA

#endif // __DAVAENGINE_SERVICEREGISTRAR_H__
