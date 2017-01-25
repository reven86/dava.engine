#include <Functional/Function.h>
#include <Debug/DVAssert.h>
#include <Network/NetCore.h>
#include <Network/NetConfig.h>
#include <Network/Private/NetController.h>
#include <Network/Private/Announcer.h>
#include <Network/Private/Discoverer.h>

#include "Engine/Engine.h"

namespace DAVA
{
namespace Net
{
const char8 NetCore::defaultAnnounceMulticastGroup[] = "239.192.100.1";

#if defined(__DAVAENGINE_COREV2__)
NetCore::NetCore(Engine* e)
    : engine(e)
    , loop(true)
    , isFinishing(false)
    , allStopped(false)
{
    sigUpdateId = e->update.Connect(this, &NetCore::Poll);
}
#else
NetCore::NetCore()
    : loop(true)
    , isFinishing(false)
    , allStopped(false)
{
}
#endif

NetCore::~NetCore()
{
#if defined(__DAVAENGINE_COREV2__)
    engine->update.Disconnect(sigUpdateId);
#endif

    DVASSERT(true == trackedObjects.empty() && true == dyingObjects.empty());
}

NetCore::TrackId NetCore::CreateController(const NetConfig& config, void* context, uint32 readTimeout)
{
#if !defined(DAVA_NETWORK_DISABLE)
    DVASSERT(false == isFinishing && true == config.Validate());
    NetController* ctrl = new NetController(&loop, registrar, context, readTimeout);
    if (true == ctrl->ApplyConfig(config))
    {
        loop.Post(Bind(&NetCore::DoStart, this, ctrl));
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
    DVASSERT(false == isFinishing);
    Announcer* ctrl = new Announcer(&loop, endpoint, sendPeriod, needDataCallback, tcpEndpoint);
    loop.Post(Bind(&NetCore::DoStart, this, ctrl));
    return ObjectToTrackId(ctrl);
#else
    return INVALID_TRACK_ID;
#endif
}

NetCore::TrackId NetCore::CreateDiscoverer(const Endpoint& endpoint, Function<void(size_t, const void*, const Endpoint&)> dataReadyCallback)
{
#if !defined(DAVA_NETWORK_DISABLE)
    DVASSERT(false == isFinishing);
    Discoverer* ctrl = new Discoverer(&loop, endpoint, dataReadyCallback);
    discovererId = ObjectToTrackId(ctrl);
    loop.Post(Bind(&NetCore::DoStart, this, ctrl));
    return discovererId;
#else
    return INVALID_TRACK_ID;
#endif
}

void NetCore::DestroyController(TrackId id)
{
#if !defined(DAVA_NETWORK_DISABLE)
    DVASSERT(false == isFinishing);
    DVASSERT(GetTrackedObject(id) != NULL);
    if (id == discovererId)
    {
        discovererId = INVALID_TRACK_ID;
    }
    loop.Post(Bind(&NetCore::DoDestroy, this, id, nullptr));
#endif
}

void NetCore::DestroyControllerBlocked(TrackId id)
{
#if !defined(DAVA_NETWORK_DISABLE)
    DVASSERT(false == isFinishing);
    DVASSERT(GetTrackedObject(id) != nullptr);

    volatile bool oneStopped = false;
    loop.Post(Bind(&NetCore::DoDestroy, this, id, &oneStopped));

    // Block until given controller is stopped and destroyed
    do
    {
        Poll();
    } while (!oneStopped);
#endif
}

void NetCore::DestroyAllControllers(Function<void()> callback)
{
#if !defined(DAVA_NETWORK_DISABLE)
    DVASSERT(false == isFinishing && controllersStoppedCallback == nullptr);

    controllersStoppedCallback = callback;
    loop.Post(MakeFunction(this, &NetCore::DoDestroyAll));
#endif
}

void NetCore::DestroyAllControllersBlocked()
{
#if !defined(DAVA_NETWORK_DISABLE)
    DVASSERT(false == isFinishing && false == allStopped && controllersStoppedCallback == nullptr);
    loop.Post(MakeFunction(this, &NetCore::DoDestroyAll));

    // Block until all controllers are stopped and destroyed
    do
    {
        Poll();
    } while (false == allStopped);
    allStopped = false;
#endif
}

void NetCore::RestartAllControllers()
{
#if !defined(DAVA_NETWORK_DISABLE)
    // Restart controllers on mobile devices
    loop.Post(MakeFunction(this, &NetCore::DoRestart));
#endif
}

void NetCore::Finish(bool runOutLoop)
{
#if !defined(DAVA_NETWORK_DISABLE)
    isFinishing = true;
    loop.Post(MakeFunction(this, &NetCore::DoDestroyAll));
    if (runOutLoop)
        loop.Run(IOLoop::RUN_DEFAULT);
#endif
}

bool NetCore::TryDiscoverDevice(const Endpoint& endpoint)
{
#if !defined(DAVA_NETWORK_DISABLE)
    if (discovererId != INVALID_TRACK_ID)
    {
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
    trackedObjects.insert(ctrl);
    ctrl->Start();
}

void NetCore::DoRestart()
{
    for (Set<IController *>::iterator i = trackedObjects.begin(), e = trackedObjects.end(); i != e; ++i)
    {
        IController* ctrl = *i;
        ctrl->Restart();
    }
}

void NetCore::DoDestroy(TrackId id, volatile bool* stoppedFlag)
{
    DVASSERT(GetTrackedObject(id) != NULL);
    IController* ctrl = GetTrackedObject(id);
    if (trackedObjects.erase(ctrl) > 0)
    {
        dyingObjects.insert(ctrl);
        ctrl->Stop(Bind(&NetCore::TrackedObjectStopped, this, _1, stoppedFlag));
    }
    else if (stoppedFlag != nullptr)
        *stoppedFlag = true;
}

void NetCore::DoDestroyAll()
{
    for (Set<IController *>::iterator i = trackedObjects.begin(), e = trackedObjects.end(); i != e; ++i)
    {
        IController* ctrl = *i;
        dyingObjects.insert(ctrl);
        ctrl->Stop(Bind(&NetCore::TrackedObjectStopped, this, _1, nullptr));
    }
    trackedObjects.clear();

    if (true == dyingObjects.empty())
    {
        AllDestroyed();
    }
}

void NetCore::AllDestroyed()
{
    allStopped = true;
    if (controllersStoppedCallback != nullptr)
    {
        controllersStoppedCallback();
        controllersStoppedCallback = nullptr;
    }
    if (true == isFinishing)
    {
        loop.PostQuit();
    }
}

IController* NetCore::GetTrackedObject(TrackId id) const
{
    DVASSERT(trackedObjects.size() != 0);

    Set<IController*>::const_iterator i = trackedObjects.find(TrackIdToObject(id));
    return (i != trackedObjects.end()) ? *i : nullptr;
}

void NetCore::TrackedObjectStopped(IController* obj, volatile bool* stoppedFlag)
{
    DVASSERT(dyingObjects.find(obj) != dyingObjects.end());
    if (dyingObjects.erase(obj) > 0) // erase returns number of erased elements
    {
        SafeDelete(obj);
        if (stoppedFlag != nullptr)
        {
            *stoppedFlag = true;
        }
    }

    if (true == dyingObjects.empty() && true == trackedObjects.empty())
    {
        AllDestroyed();
    }
}

} // namespace Net
} // namespace DAVA
