#include "Network/NetCore.h"
#include "Network/NetConfig.h"
#include "Network/Private/NetController.h"
#include "Network/Private/Announcer.h"
#include "Network/Private/Discoverer.h"
#include "Functional/Function.h"
#include "Debug/DVAssert.h"
#include "Engine/Engine.h"
#include "Base/BaseTypes.h"
#include "FileSystem/KeyedArchive.h"
#include "Concurrency/LockGuard.h"

namespace DAVA
{
namespace Net
{
const char8 NetCore::defaultAnnounceMulticastGroup[] = "239.192.100.1";

#if defined(__DAVAENGINE_COREV2__)
NetCore::NetCore(Engine* e)
    : engine(e)
    , loopCreatedEvent(false)
{
    bool separateThreadDefaultValue = false;
    const KeyedArchive* options = e->GetOptions();
    useSeparateThread = options->GetBool("separate_net_thread", separateThreadDefaultValue);

    e->update.Connect(this, &NetCore::Update);

    netEventsDispatcher.reset(new Dispatcher<Function<void()>>([](const Function<void()>& fn) { fn(); }));
    netEventsDispatcher->LinkToCurrentThread();

    if (useSeparateThread)
    {
        netThread = Thread::Create([this]() { NetThreadHandler(); });
        netThread->Start();
        loopCreatedEvent.Wait();
    }
    else
    {
        loop = new IOLoop(true);
    }

#if defined(__DAVAENGINE_IPHONE__)
    // iOS silently kills sockets when device is locked so recreate sockets
    // when application is resumed
    e->resumed.Connect(this, &NetCore::RestartAllControllers);
#endif
}
#else
NetCore::NetCore()
{
    loop = new IOLoop(true);
}
#endif

NetCore::~NetCore()
{
#if defined(__DAVAENGINE_COREV2__)
    engine->update.Disconnect(this);
#if defined(__DAVAENGINE_IPHONE__)
    engine->resumed.Disconnect(this);
#endif
#endif

    Finish(true);

    DVASSERT(state == State::FINISHED);
    if (netThread)
    {
        DVASSERT(netThread->GetState() == Thread::eThreadState::STATE_ENDED);
    }

    DVASSERT(true == trackedObjects.empty());
    DVASSERT(true == dyingObjects.empty());

    if (!useSeparateThread)
    {
        SafeDelete(loop);
    }
}

void NetCore::NetThreadHandler()
{
    loop = new IOLoop(true);
    loopCreatedEvent.Signal();
    loop->Run();
    SafeDelete(loop);
}

void NetCore::Update(float32)
{
    ProcessPendingEvents();
    if (!useSeparateThread)
    {
        Poll();
    }
}

void NetCore::ProcessPendingEvents()
{
    if (netEventsDispatcher->HasEvents())
    {
        netEventsDispatcher->ProcessEvents();
    }
}

Dispatcher<Function<void()>>* NetCore::GetNetEventsDispatcher()
{
    return netEventsDispatcher.get();
}

NetCore::TrackId NetCore::CreateController(const NetConfig& config, void* context, uint32 readTimeout)
{
#if !defined(DAVA_NETWORK_DISABLE)
    DVASSERT(state == State::ACTIVE && true == config.Validate());
    NetController* ctrl = new NetController(loop, registrar, context, readTimeout);
    if (true == ctrl->ApplyConfig(config))
    {
        loop->Post(Bind(&NetCore::DoStart, this, ctrl));
        return ObjectToTrackId(ctrl);
    }
    else
    {
        delete ctrl;
        return INVALID_TRACK_ID;
    }
#else
    return INVALID_TRACK_ID;
#endif
}

NetCore::TrackId NetCore::CreateAnnouncer(const Endpoint& endpoint, uint32 sendPeriod, Function<size_t(size_t, void*)> needDataCallback, const Endpoint& tcpEndpoint)
{
#if !defined(DAVA_NETWORK_DISABLE)
    DVASSERT(state == State::ACTIVE);
    Announcer* ctrl = new Announcer(loop, endpoint, sendPeriod, needDataCallback, tcpEndpoint);
    loop->Post(Bind(&NetCore::DoStart, this, ctrl));
    return ObjectToTrackId(ctrl);
#else
    return INVALID_TRACK_ID;
#endif
}

NetCore::TrackId NetCore::CreateDiscoverer(const Endpoint& endpoint, Function<void(size_t, const void*, const Endpoint&)> dataReadyCallback)
{
#if !defined(DAVA_NETWORK_DISABLE)
    DVASSERT(state == State::ACTIVE);
    Discoverer* ctrl = new Discoverer(loop, endpoint, dataReadyCallback);
    discovererId = ObjectToTrackId(ctrl);
    loop->Post(Bind(&NetCore::DoStart, this, ctrl));
    return discovererId;
#else
    return INVALID_TRACK_ID;
#endif
}

void NetCore::DestroyController(TrackId id, Function<void()> callback)
{
#if !defined(DAVA_NETWORK_DISABLE)
    DVASSERT(state == State::ACTIVE);

    IController* ctrl = GetTrackedObject(id);
    DVASSERT(ctrl != nullptr);

    if (id == discovererId)
    {
        discovererId = INVALID_TRACK_ID;
    }

    if (callback)
    {
        controllerStoppedCallback.emplace(TrackIdToObject(id), callback);
    }

    size_t erased = 0;
    {
        LockGuard<Mutex> lock(trackedObjectsMutex);
        erased = trackedObjects.erase(ctrl);
    }

    if (erased > 0)
    {
        {
            LockGuard<Mutex> lock(dyingObjectsMutex);
            dyingObjects.emplace(ctrl);
        }
        loop->Post(Bind(&NetCore::DoDestroy, this, ctrl));
    }
#endif
}

void NetCore::DestroyControllerBlocked(TrackId id)
{
#if !defined(DAVA_NETWORK_DISABLE)
    DVASSERT(state == State::ACTIVE);

    IController* ctrl = GetTrackedObject(id);
    DVASSERT(ctrl != nullptr);

    size_t erased = 0;
    {
        LockGuard<Mutex> lock(trackedObjectsMutex);
        erased = trackedObjects.erase(ctrl);
    }

    if (erased > 0)
    {
        {
            LockGuard<Mutex> lock(dyingObjectsMutex);
            dyingObjects.emplace(ctrl);
        }
        loop->Post(Bind(&NetCore::DoDestroy, this, ctrl));

        WaitForDestroyed(ctrl);
    }
#endif
}

void NetCore::WaitForDestroyed(IController* ctrl)
{
    while (true)
    {
        {
            LockGuard<Mutex> lock(dyingObjectsMutex);
            if (dyingObjects.find(ctrl) == dyingObjects.end())
            {
                break;
            }
        }

        Update();
    }
}

bool NetCore::PostAllToDestroy()
{
    LockGuard<Mutex> lock(dyingObjectsMutex);
    LockGuard<Mutex> lock2(trackedObjectsMutex);

    bool hasControllersToDestroy = !trackedObjects.empty();
    for (IController* ctrl : trackedObjects)
    {
        dyingObjects.emplace(ctrl);
        loop->Post(Bind(&NetCore::DoDestroy, this, ctrl));
    }
    trackedObjects.clear();

    return hasControllersToDestroy;
}

void NetCore::DestroyAllControllers(Function<void()> callback)
{
#if !defined(DAVA_NETWORK_DISABLE)
    DVASSERT(state == State::ACTIVE);
    DVASSERT(allControllersStoppedCallback == nullptr);

    allControllersStoppedCallback = callback;
    PostAllToDestroy();
#endif
}

void NetCore::DestroyAllControllersBlocked()
{
#if !defined(DAVA_NETWORK_DISABLE)
    DVASSERT(state == State::ACTIVE);
    DVASSERT(allControllersStoppedCallback == nullptr);

    PostAllToDestroy();
    WaitForAllDestroyed();
#endif
}

void NetCore::WaitForAllDestroyed()
{
    while (true)
    {
        {
            LockGuard<Mutex> lock(dyingObjectsMutex);
            if (dyingObjects.empty())
                break;
        }

        Update();
    }
}

void NetCore::RestartAllControllers()
{
#if !defined(DAVA_NETWORK_DISABLE)
    // Restart controllers on mobile devices
    loop->Post(MakeFunction(this, &NetCore::DoRestart));
#endif
}

void NetCore::Finish(bool waitForFinished)
{
#if !defined(DAVA_NETWORK_DISABLE)

    if (state == State::ACTIVE)
    {
        state = State::FINISHING;
        bool hasControllersToDestroy = PostAllToDestroy();

        if (!hasControllersToDestroy)
        {
            AllDestroyed();
        }

        if (waitForFinished)
        {
            if (hasControllersToDestroy)
            {
                WaitForAllDestroyed();
            }

            if (useSeparateThread)
            {
                netThread->Join();
            }
            else
            {
                loop->Run(IOLoop::RUN_DEFAULT);
            }
        }
    }
#endif
}

bool NetCore::TryDiscoverDevice(const Endpoint& endpoint)
{
#if !defined(DAVA_NETWORK_DISABLE)
    if (discovererId != INVALID_TRACK_ID)
    {
        LockGuard<Mutex> lock(trackedObjectsMutex);
        auto it = trackedObjects.find(TrackIdToObject(discovererId));
        if (it != trackedObjects.end())
        {
            // Variable is named in honor of big fan and donater of tanks - Sergey Demidov
            // And this man assures that cast below is valid, so do not worry, guys
            Discoverer* SergeyDemidov = static_cast<Discoverer*>(*it);
            return SergeyDemidov->TryDiscoverDevice(endpoint);
        }
    }
    return false;
#else
    return true;
#endif
}

void NetCore::DoStart(IController* ctrl)
{
    LockGuard<Mutex> lock(trackedObjectsMutex);
    trackedObjects.insert(ctrl);
    ctrl->Start();
}

void NetCore::DoRestart()
{
    LockGuard<Mutex> lock(trackedObjectsMutex);
    for (IController* ctrl : trackedObjects)
    {
        ctrl->Restart();
    }
}

void NetCore::DoDestroy(IController* ctrl)
{
    DVASSERT(ctrl != nullptr);
    ctrl->Stop(Bind(&NetCore::TrackedObjectStopped, this, _1));
}

void NetCore::AllDestroyed()
{
    if (allControllersStoppedCallback != nullptr)
    {
        allControllersStoppedCallback();
        allControllersStoppedCallback = nullptr;
    }
    if (state == State::FINISHING)
    {
        state = State::FINISHED;
        loop->PostQuit();
    }
}

IController* NetCore::GetTrackedObject(TrackId id)
{
    LockGuard<Mutex> lock(trackedObjectsMutex);
    DVASSERT(trackedObjects.empty() == false);

    Set<IController*>::const_iterator i = trackedObjects.find(TrackIdToObject(id));
    return (i != trackedObjects.end()) ? *i : nullptr;
}

void NetCore::TrackedObjectStopped(IController* obj)
{
    Function<void()> callbackOnStopped;
    bool allDestroyed = false;

    {
        LockGuard<Mutex> lock(dyingObjectsMutex);
        LockGuard<Mutex> lock2(trackedObjectsMutex);

        if (dyingObjects.erase(obj) == 0)
        {
            DVASSERT(false && "dying object is not found");
        }

        auto cbkFound = controllerStoppedCallback.find(obj);
        if (cbkFound != controllerStoppedCallback.end())
        {
            callbackOnStopped = cbkFound->second;
            controllerStoppedCallback.erase(cbkFound);
        }

        allDestroyed = (true == dyingObjects.empty() && true == trackedObjects.empty());
    }

    if (callbackOnStopped)
    {
        callbackOnStopped();
    }

    if (allDestroyed)
    {
        AllDestroyed();
    }
}

size_t NetCore::ControllersCount() const
{
    LockGuard<Mutex> lock(trackedObjectsMutex);
    return trackedObjects.size();
}

} // namespace Net
} // namespace DAVA
