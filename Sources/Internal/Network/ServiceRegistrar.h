#ifndef __DAVAENGINE_SERVICEREGISTRAR_H__
#define __DAVAENGINE_SERVICEREGISTRAR_H__

#include <Base/BaseTypes.h>
#include <Functional/Function.h>
#include "Logger/Logger.h"
#include "Concurrency/Thread.h"
#include "Network/NetEventsDispatcher.h"

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
        targetFnResult = targetFn(serviceId, context);
    }

    uint32 serviceId = 0;
    void* context = nullptr;
    ServiceCreator targetFn;
    IChannelListener* targetFnResult = nullptr;
};

class ServiceCreatorAsync
{
public:
    explicit ServiceCreatorAsync(ServiceCreator targetFn, NetEventsDispatcher* dispatcher)
        : targetFn(targetFn)
        , dispatcher(dispatcher)
    {
    }

    IChannelListener* ServiceCreatorCall(uint32 serviceId, void* context)
    {
        std::shared_ptr<ServiceCreatorExecutor> executor(new ServiceCreatorExecutor(targetFn, serviceId, context));
        std::weak_ptr<ServiceCreatorExecutor> executorWeak = executor;
        auto fn = MakeFunction(executorWeak, &ServiceCreatorExecutor::ServiceCreatorCall);
        dispatcher->SendEvent(fn);
        return executor->targetFnResult;
    }

    ServiceCreator targetFn;
    NetEventsDispatcher* dispatcher = nullptr;
};

class ServiceDeleterAsync
{
public:
    explicit ServiceDeleterAsync(ServiceDeleter targetFn, NetEventsDispatcher* holder)
        : targetFn(targetFn)
        , holder(holder)
    {
    }

    void ServiceDeleterCall(IChannelListener* obj, void* context)
    {
        auto fn = Bind(targetFn, obj, context);
        holder->SendEvent(fn);
    }

    ServiceDeleter targetFn;
    NetEventsDispatcher* holder = nullptr;
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
