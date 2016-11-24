#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_COREV2__)

#include "Concurrency/Mutex.h"
#include "Concurrency/LockGuard.h"
#include "Concurrency/Semaphore.h"
#include "Concurrency/AutoResetEvent.h"
#include "Concurrency/Thread.h"
#include "Functional/Function.h"

#include "Debug/DeadlockMonitor.h"

#include <atomic>

namespace DAVA
{
/**
    \ingroup engine
    Template class that implements event dispatcher where template parameter T specifies event type.
    T must be copy or move constructible.

    Dispatcher manages event queue and provides methods to place events to queue and extract events from queue.
    Application can place events from any thread, but extraction **must** be performed only from single thread
    which should stay the same until dispatcher dies. Event extraction thread is set by `LinkToCurrentThread` method.
*/
template <typename T>
class Dispatcher final
{
public:
    /**
        Dispatcher constructor

        \param handler Function object which will be invoked for each event when application calls `ProcessEvents`
        \param trigger Function object to trigger (initiate) blocking call
    */
    Dispatcher(const Function<void(const T&)>& handler, const Function<void()> trigger = []() {});
    ~Dispatcher() = default;

    // Explicitly delete copy and move assignment only for msvc 2013
    // as it does not fully conform c++11
    Dispatcher(const Dispatcher&) = delete;
    Dispatcher& operator=(const Dispatcher&) = delete;

    Dispatcher(Dispatcher&&) = delete;
    Dispatcher& operator=(Dispatcher&&) = delete;

    /** Set thread where events are processed by application */
    void LinkToCurrentThread();
    uint64 GetLinkedThread() const;

    /** Check whether dispatcher has any events */
    bool HasEvents() const;

    /**
        Place event into queue and immediately return (do not wait event procession).
    */
    template <typename U>
    void PostEvent(U&& e);

    /**
        Place event into queue and wait until event is processed.
    */
    template <typename U>
    void SendEvent(U&& e);

    /**
        Process events that are currently in queue. For each event in queue dispatcher
        invokes `handler` set in dispatcher constructor.
    */
    void ProcessEvents();

    /**
        Examine event queue, for each event `viewer` is invoked.
    */
    void ViewEventQueue(const Function<void(const T&)>& viewer);

private:
    struct EventWrapper
    {
        EventWrapper(const T& x, AutoResetEvent* sigEvent)
            : e(x)
            , signalEvent(sigEvent)
        {
        }

        T e;
        AutoResetEvent* signalEvent = nullptr;
    };
    using EventType = EventWrapper;

    mutable Mutex mutex; // Mutex to guard event queue
    Vector<EventType> eventQueue;
    Vector<EventType> readyEvents;
    Function<void(const T&)> eventHandler;
    Function<void()> sendEventTrigger;
    size_t curEventIndex = 0;

    static const int poolSize = 2;

    uint64 linkedThreadId = 0; // Identifier of thread that calls Dispatcher::ProcessEvents method
    Semaphore semaphore; // Semaphore to guard signal event pool
    AutoResetEvent signalEventPool[poolSize]; // Pool of events to signal about blocking call completion
    std::atomic_flag signalEventBusyFlag[poolSize]; // Flags that indicate what signal events are busy
};

template <typename T>
Dispatcher<T>::Dispatcher(const Function<void(const T&)>& handler, const Function<void()> trigger)
    : eventHandler(handler)
    , sendEventTrigger(trigger)
    , semaphore(poolSize)
{
    for (std::atomic_flag& f : signalEventBusyFlag)
    {
        f.clear();
    }
}

template <typename T>
void Dispatcher<T>::LinkToCurrentThread()
{
    linkedThreadId = Thread::GetCurrentIdAsUInt64();
}

template <typename T>
uint64 Dispatcher<T>::GetLinkedThread() const
{
    return linkedThreadId;
}

template <typename T>
bool Dispatcher<T>::HasEvents() const
{
    LockGuard<Mutex> lock(mutex);
    return !eventQueue.empty();
}

template <typename T>
template <typename U>
void Dispatcher<T>::PostEvent(U&& e)
{
    LockGuard<Mutex> lock(mutex);
    eventQueue.emplace_back(std::forward<U>(e), nullptr);
}

template <typename T>
template <typename U>
void Dispatcher<T>::SendEvent(U&& e)
{
    DVASSERT_MSG(linkedThreadId != 0, "Before calling SendEvent you must call LinkToCurrentThread");

    uint64 curThreadId = Thread::GetCurrentIdAsUInt64();
    if (linkedThreadId == curThreadId)
    {
        // If blocking call is made from the same thread as thread that calls ProcessEvents
        // simply call ProcessEvents
        {
            LockGuard<Mutex> lock(mutex);
            eventQueue.emplace_back(std::forward<U>(e), nullptr);
        }
        ProcessEvents();
    }
    else
    {
        int signalEventIndex = -1;
        while (signalEventIndex < 0)
        {
            for (int i = 0; i < poolSize; ++i)
            {
                if (!signalEventBusyFlag[i].test_and_set(std::memory_order_acquire))
                {
                    signalEventIndex = i;
                    break;
                }
            }
            if (signalEventIndex < 0)
            {
                // Wait till any signal event is released
                semaphore.Wait();
            }
        }

        {
            LockGuard<Mutex> lock(mutex);
            eventQueue.emplace_back(std::forward<U>(e), &signalEventPool[signalEventIndex]);
        }

        DAVA_BEGIN_BLOCKING_CALL(linkedThreadId);

        sendEventTrigger();

        signalEventPool[signalEventIndex].Wait();
        signalEventBusyFlag[signalEventIndex].clear(std::memory_order_release);
        semaphore.Post(1);

        DAVA_END_BLOCKING_CALL(linkedThreadId);
    }
}

template <typename T>
void Dispatcher<T>::ProcessEvents()
{
    {
        LockGuard<Mutex> lock(mutex);
        eventQueue.swap(readyEvents);
    }

    curEventIndex = 0;
    for (const EventType& w : readyEvents)
    {
        eventHandler(w.e);
        if (w.signalEvent != nullptr)
        {
            w.signalEvent->Signal();
        }
        curEventIndex += 1;
    }
    readyEvents.clear();
}

template <typename T>
void Dispatcher<T>::ViewEventQueue(const Function<void(const T&)>& viewer)
{
    for (size_t i = curEventIndex + 1, n = readyEvents.size(); i < n; ++i)
    {
        viewer(readyEvents[i].e);
    }
}

} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
