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

    DVASSERT(true == controllers.empty());

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
    std::unique_ptr<NetController> ctrl(new NetController(loop, registrar, context, readTimeout));
    TrackId id = ObjectToTrackId(ctrl.get());
    if (true == ctrl->ApplyConfig(config))
    {
        IController* c = ctrl.get();
        {
            LockGuard<Mutex> lock(controllersMutex);
            ControllerContext& context = controllers[id];
            context.status = ControllerContext::STARTING;
            context.ctrl = std::move(ctrl);
        }
        loop->Post(Bind(&NetCore::DoStart, this, c));
        return id;
    }
    else
    {
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

    if (id == discovererId)
    {
        discovererId = INVALID_TRACK_ID;
    }

    bool isStartingController = false;

    {
        LockGuard<Mutex> lock(controllersMutex);
        auto it = controllers.find(id);
        if (it != controllers.end())
        {
            ControllerContext& context = it->second;
            if (context.status == ControllerContext::STARTING)
            {
                isStartingController = true;
            }
            else if (context.status == ControllerContext::RUNNING)
            {
                context.status = ControllerContext::DESTROYING;
                context.controllerStoppedCallback = callback;
                loop->Post(Bind(&NetCore::DoDestroy, this, context.ctrl.get()));
            }
        }
        else
        {
            DVASSERT(false, "passed controller is not found");
        }
    }

    if (isStartingController)
    {
        if (callback)
        {
            callback();
        }

        LockGuard<Mutex> lock(controllersMutex);
        controllers.erase(id);
    }
#endif
}

void NetCore::DestroyControllerBlocked(TrackId id)
{
#if !defined(DAVA_NETWORK_DISABLE)
    DVASSERT(state == State::ACTIVE);

    bool toWait = false;
    {
        LockGuard<Mutex> lock(controllersMutex);
        auto it = controllers.find(id);
        if (it != controllers.end())
        {
            ControllerContext& context = it->second;
            if (context.status == ControllerContext::STARTING)
            {
                controllers.erase(it);
            }
            else
            {
                toWait = true;
                if (context.status == ControllerContext::RUNNING)
                {
                    context.status = ControllerContext::DESTROYING;
                    loop->Post(Bind(&NetCore::DoDestroy, this, context.ctrl.get()));
                }
            }
        }
        else
        {
            DVASSERT(false, "passed controller is not found");
            return;
        }
    }

    if (toWait)
    {
        WaitForDestroyed(id);
    }
#endif
}

void NetCore::WaitForDestroyed(TrackId id)
{
    while (true)
    {
        {
            LockGuard<Mutex> lock(controllersMutex);
            if (controllers.find(id) == controllers.end())
            {
                break;
            }
        }

        Update();
    }
}

bool NetCore::PostAllToDestroy()
{
    LockGuard<Mutex> lock(controllersMutex);

    bool hasControllersToDestroy = false;
    for (auto it = controllers.begin(); it != controllers.end();)
    {
        ControllerContext& context = it->second;
        if (context.status == ControllerContext::STARTING)
        {
            auto itDel = it++;
            controllers.erase(it);
        }
        else
        {
            ++it;
            if (context.status == ControllerContext::RUNNING)
            {
                loop->Post(Bind(&NetCore::DoDestroy, this, context.ctrl.get()));
            }
            hasControllersToDestroy = true;
        }
    }

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
            LockGuard<Mutex> lock(controllersMutex);
            if (controllers.empty())
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
        LockGuard<Mutex> lock(controllersMutex);
        auto it = controllers.find(discovererId);
        if (it != controllers.end() && it->second.status == ControllerContext::RUNNING)
        {
            IController* ctrl = it->second.ctrl.get();
            // Variable is named in honor of big fan and donater of tanks - Sergey Demidov
            // And this man assures that cast below is valid, so do not worry, guys
            Discoverer* SergeyDemidov = static_cast<Discoverer*>(ctrl);
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
    bool toStart = false;
    {
        LockGuard<Mutex> lock(controllersMutex);
        auto it = controllers.find(ObjectToTrackId(ctrl));
        if (it != controllers.end())
        {
            if (it->second.status == ControllerContext::STARTING)
            {
                toStart = true;
                it->second.status = ControllerContext::RUNNING;
            }
            else
            {
                DVASSERT("Controller status is not STARTING");
                return;
            }
        }
    }

    if (toStart)
    {
        ctrl->Start();
    }
}

void NetCore::DoRestart()
{
    LockGuard<Mutex> lock(controllersMutex);
    for (auto& entry : controllers)
    {
        if (entry.second.status == ControllerContext::RUNNING)
            entry.second.ctrl->Restart();
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

void NetCore::TrackedObjectStopped(IController* obj)
{
    Function<void()> callbackOnStopped;
    bool allDestroyed = false;

    {
        LockGuard<Mutex> lock(controllersMutex);
        auto it = controllers.find(ObjectToTrackId(obj));
        if (it != controllers.end())
        {
            DVASSERT(it->second.status == ControllerContext::DESTROYING);
            callbackOnStopped = it->second.controllerStoppedCallback;
            controllers.erase(it);
        }
        else
        {
            DVASSERT(false && "stopped controller is not found in map");
        }

        allDestroyed = (true == controllers.empty());
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
    LockGuard<Mutex> lock(controllersMutex);
    return controllers.size();
}

} // namespace Net
} // namespace DAVA
