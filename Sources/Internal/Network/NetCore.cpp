#include <Functional/Function.h>
#include <Debug/DVAssert.h>
#include <Network/NetCore.h>
#include <Network/NetConfig.h>
#include <Network/Private/NetController.h>
#include <Network/Private/Announcer.h>
#include <Network/Private/Discoverer.h>
#include <Engine/Engine.h>
#include <Base/BaseTypes.h>
#include <FileSystem/KeyedArchive.h>
#include <Concurrency/LockGuard.h>

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
    bool separateThreadDefaultValue = false;
    const KeyedArchive* options = e->GetOptions();
    useSeparateThread = options->GetBool("separate_net_thread", separateThreadDefaultValue);

    sigUpdateId = e->update.Connect(this, &NetCore::OnEngineUpdate);

    NetCallbacksHolder::Mode mode = (useSeparateThread ? NetCallbacksHolder::AddInQueue : NetCallbacksHolder::ExecuteImmediately);
    netCallbacksHolder.reset(new NetCallbacksHolder(mode));

    if (useSeparateThread)
    {
        netThread = Thread::Create([this]() { NetThreadHandler(); });
        netThread->Start();
    }

#if defined(__DAVAENGINE_IPHONE__)
    // iOS silently kills sockets when device is locked so recreate sockets
    // when application is resumed
    sigResumedId = e->resumed.Connect(this, &NetCore::RestartAllControllers);
#endif
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
#if defined(__DAVAENGINE_IPHONE__)
    engine->resumed.Disconnect(sigResumedId);
#endif
#endif

    DVASSERT(true == trackedObjects.empty() && true == dyingObjects.empty());
}

void NetCore::NetThreadHandler()
{
    loop.Run();
}

void NetCore::OnEngineUpdate(float32)
{
    if (useSeparateThread)
        ExecPendingCallbacks();
    else
        Poll();
}

void NetCore::ExecPendingCallbacks()
{
    netCallbacksHolder->ExecPendingCallbacks();
}

NetCallbacksHolder* NetCore::GetNetCallbacksHolder()
{
    return netCallbacksHolder.get();
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
    loop.Post(Bind(&NetCore::DoDestroy, this, GetTrackedObject(id)));
#endif
}

void NetCore::DestroyControllerBlocked(TrackId id)
{
#if !defined(DAVA_NETWORK_DISABLE)
    DVASSERT(false == isFinishing);

    IController* ctrl = GetTrackedObject(id);
    DVASSERT(ctrl != nullptr);

    if (trackedObjects.erase(ctrl) > 0)
    {
        {
            LockGuard<Mutex> lock(dyingObjectsMutex);
            auto emplaceRes = dyingObjects.emplace(ctrl);
        }
        loop.Post(Bind(&NetCore::DoDestroy, this, ctrl));

        while (true)
        {
            {
                LockGuard<Mutex> lock(dyingObjectsMutex);
                if (dyingObjects.find(ctrl) == dyingObjects.end())
                {
                    break;
                }
            }

            if (useSeparateThread)
                ExecPendingCallbacks();
            else
                Poll();
        }
    }
#endif
}

bool NetCore::PostAllToDestroy()
{
    LockGuard<Mutex> lock(dyingObjectsMutex);
    bool hasControllersToDestroy = !trackedObjects.empty();
    for (IController* ctrl : trackedObjects)
    {
        dyingObjects.emplace(ctrl);
        loop.Post(Bind(&NetCore::DoDestroy, this, ctrl));
    }
    trackedObjects.clear();

    return hasControllersToDestroy;
}

void NetCore::DestroyAllControllers(Function<void()> callback)
{
#if !defined(DAVA_NETWORK_DISABLE)
    DVASSERT(false == isFinishing && controllersStoppedCallback == nullptr);

    controllersStoppedCallback = callback;
    PostAllToDestroy();
#endif
}

void NetCore::DestroyAllControllersBlocked()
{
#if !defined(DAVA_NETWORK_DISABLE)
    DVASSERT(false == isFinishing && controllersStoppedCallback == nullptr);

    PostAllToDestroy();

    while (true)
    {
        LockGuard<Mutex> lock(dyingObjectsMutex);
        if (dyingObjects.empty())
            break;

        if (useSeparateThread)
            ExecPendingCallbacks();
        else
            Poll();
    }
#endif
}

void NetCore::RestartAllControllers()
{
#if !defined(DAVA_NETWORK_DISABLE)
    // Restart controllers on mobile devices
    loop.Post(MakeFunction(this, &NetCore::DoRestart));
#endif
}

void NetCore::Finish(bool doBlocked)
{
#if !defined(DAVA_NETWORK_DISABLE)

    isFinishing = true;
    bool hasControllersToDestroy = PostAllToDestroy();

    if (hasControllersToDestroy)
    {
        if (doBlocked)
        {
            if (useSeparateThread)
            {
                while (true)
                {
                    LockGuard<Mutex> lock(dyingObjectsMutex);
                    if (dyingObjects.empty())
                        break;

                    ExecPendingCallbacks();
                }
                netThread->Join();
            }
            else
            {
                loop.Run(IOLoop::RUN_DEFAULT);
            }
        }
    }
    else
    {
        AllDestroyed();
        if (useSeparateThread)
        {
            netThread->Join();
        }
        else
        {
            loop.Run(IOLoop::RUN_DEFAULT);
        }
    }
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
            // And this man assures that cast below is valid, so do not worry,  guys
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

void NetCore::TrackedObjectStopped(IController* obj)
{
    LockGuard<Mutex> lock(dyingObjectsMutex);
    if (dyingObjects.erase(obj) == 0)
    {
        DVASSERT(false && "dying object is not found");
    }

    if (true == dyingObjects.empty() && true == trackedObjects.empty())
    {
        AllDestroyed();
    }
}

} // namespace Net
} // namespace DAVA
