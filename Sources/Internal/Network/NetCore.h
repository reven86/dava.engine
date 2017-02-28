#pragma once

#include "Base/BaseTypes.h"
#include "Functional/Function.h"
#include "Base/Singleton.h"

#include "Network/Base/IOLoop.h"
#include "Network/Base/IfAddress.h"
#include "Network/Base/Endpoint.h"
#include "Network/ServiceRegistrar.h"
#include "Network/IController.h"
#include "Network/NetworkCommon.h"
#include "Network/NetEventsDispatcher.h"
#include "Concurrency/Thread.h"

namespace DAVA
{
class Engine;
namespace Net
{
class NetConfig;

class NetCore : public Singleton<NetCore>
{
public:
    using TrackId = uintptr_t;
    static const TrackId INVALID_TRACK_ID = 0;

    enum eDefaultPorts
    {
        DEFAULT_TCP_ANNOUNCE_PORT = 9998,
        DEFAULT_UDP_ANNOUNCE_PORT = 9998,
        DEFAULT_TCP_PORT = 9999
    };

    static const char8 defaultAnnounceMulticastGroup[];

    enum eKnownNetworkServices
    {
        SERVICE_LOG = 0,
        SERVICE_MEMPROF
    };

public:
#if defined(__DAVAENGINE_COREV2__)

    NetCore(Engine* e);
    Engine* engine = nullptr;
    size_t sigUpdateId = 0;
#if defined(__DAVAENGINE_IPHONE__)
    size_t sigResumedId = 0;
#endif
#else
    NetCore();
#endif
    ~NetCore();

    IOLoop* Loop() const;

    bool RegisterService(uint32 serviceId, ServiceCreator creator, ServiceDeleter deleter, const char8* serviceName = NULL);
    bool UnregisterService(uint32 serviceId);
    void UnregisterAllServices();
    bool IsServiceRegistered(uint32 serviceId) const;
    const char8* ServiceName(uint32 serviceId) const;

    TrackId CreateController(const NetConfig& config, void* context = nullptr, uint32 readTimeout = DEFAULT_READ_TIMEOUT);
    TrackId CreateAnnouncer(const Endpoint& endpoint, uint32 sendPeriod, Function<size_t(size_t, void*)> needDataCallback, const Endpoint& tcpEndpoint = Endpoint(DEFAULT_TCP_ANNOUNCE_PORT));
    TrackId CreateDiscoverer(const Endpoint& endpoint, Function<void(size_t, const void*, const Endpoint&)> dataReadyCallback);
    void DestroyController(TrackId id);
    void DestroyControllerBlocked(TrackId id);
    void DestroyAllControllers(Function<void()> callback);
    void DestroyAllControllersBlocked();

    void RestartAllControllers();

    size_t ControllersCount();

    int32 Run();
#if defined(__DAVAENGINE_COREV2__)
    void Poll(float32 frameDelta = 0.0f);
#else
    int32 Poll();
#endif
    void Finish(bool runOutLoop = false);

    void OnEngineUpdate(float32 frameDelta = 0.0f);

    bool TryDiscoverDevice(const Endpoint& endpoint);

    Vector<IfAddress> InstalledInterfaces() const;

    static bool IsNetworkEnabled();

    NetEventsDispatcher* GetNetCallbacksHolder();
    void ProcessPendingEvents();

private:
    void DoUpdate();

    void NetThreadHandler();
    void DoStart(IController* ctrl);
    void DoRestart();

    bool PostAllToDestroy();
    void DoDestroy(IController* ctrl);
    void AllDestroyed();
    IController* GetTrackedObject(TrackId id);
    void TrackedObjectStopped(IController* obj);

    TrackId ObjectToTrackId(const IController* obj) const;
    IController* TrackIdToObject(TrackId id) const;

private:
    IOLoop* loop = nullptr; // Heart of NetCore and network library - event loop
    std::unique_ptr<IOLoop> loopHolder;
    bool useSeparateThread = false;

    Mutex trackedObjectsMutex;
    Set<IController*> trackedObjects; // Running objects

    Mutex dyingObjectsMutex;
    Set<IController*> dyingObjects;

    ServiceRegistrar registrar; //-V730_NOINIT
    Function<void()> controllersStoppedCallback;

    enum State
    {
        ACTIVE,
        FINISHING,
        FINISHED
    };
    Atomic<State> state{ ACTIVE };

#if !defined(DAVA_NETWORK_DISABLE)
    TrackId discovererId = INVALID_TRACK_ID;
#endif
    RefPtr<Thread> netThread;
    std::unique_ptr<NetEventsDispatcher> netEventsDispatcher;
};

//////////////////////////////////////////////////////////////////////////
inline bool NetCore::RegisterService(uint32 serviceId, ServiceCreator creator, ServiceDeleter deleter, const char8* serviceName)
{
    return registrar.Register(serviceId, creator, deleter, serviceName);
}

inline bool NetCore::UnregisterService(uint32 serviceId)
{
    return registrar.UnRegister(serviceId);
}

inline void NetCore::UnregisterAllServices()
{
    registrar.UnregisterAll();
}

inline bool NetCore::IsServiceRegistered(uint32 serviceId) const
{
    return registrar.IsRegistered(serviceId);
}

inline const char8* NetCore::ServiceName(uint32 serviceId) const
{
    return registrar.Name(serviceId);
}

inline Vector<IfAddress> NetCore::InstalledInterfaces() const
{
    return IfAddress::GetInstalledInterfaces(false);
}

inline int32 NetCore::Run()
{
    return loop->Run(IOLoop::RUN_DEFAULT);
}

#if defined(__DAVAENGINE_COREV2__)
inline void NetCore::Poll(float32 /*frameDelta*/)
{
    loop->Run(IOLoop::RUN_NOWAIT);
}
#else
inline int32 NetCore::Poll()
{
    return loop->Run(IOLoop::RUN_NOWAIT);
}
#endif

inline bool NetCore::IsNetworkEnabled()
{
#if defined(DAVA_NETWORK_DISABLE)
    return false;
#else
    return true;
#endif
}

inline NetCore::TrackId NetCore::ObjectToTrackId(const IController* obj) const
{
    return reinterpret_cast<TrackId>(obj);
}

inline IController* NetCore::TrackIdToObject(TrackId id) const
{
    return reinterpret_cast<IController*>(id);
}

inline IOLoop* NetCore::Loop() const
{
    return loop;
}

} // namespace Net
} // namespace DAVA
