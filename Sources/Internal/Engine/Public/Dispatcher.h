#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"
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
template <typename T>
class Dispatcher final
{
public:
    Dispatcher(const Function<void(const T&)>& handler);
    ~Dispatcher() = default;

    Dispatcher(const Dispatcher&) = delete;
    Dispatcher& operator=(const Dispatcher&) = delete;

    Dispatcher(Dispatcher&&) = delete;
    Dispatcher& operator=(Dispatcher&&) = delete;

    void LinkToCurrentThread();
    uint64 GetLinkedThread() const;

    void PostEvent(const T& e);
    void SendEvent(const T& e);
    void ProcessEvents();

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

private:
    Mutex mutex; // Mutex to guard event queue
    Vector<EventType> eventQueue;
    Function<void(const T&)> eventHandler;

    static const int poolSize = 4;

    uint64 linkedThreadId = 0; // Identifier of thread that calls Dispatcher::ProcessEvents method
    Semaphore semaphore; // Semaphore to guard signal event pool
    AutoResetEvent signalEventPool[poolSize]; // Pool of events to signal about blocking call completion
    std::atomic_flag signalEventBusyFlag[poolSize]; // Flags that indicate what signal events are busy
};

template <typename T>
Dispatcher<T>::Dispatcher(const Function<void(const T&)>& handler)
    : eventHandler(handler)
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
    linkedThreadId = Thread::GetCurrentIdAsInteger();
}

template <typename T>
uint64 Dispatcher<T>::GetLinkedThread() const
{
    return linkedThreadId;
}

template <typename T>
void Dispatcher<T>::PostEvent(const T& e)
{
    LockGuard<Mutex> lock(mutex);
    eventQueue.emplace_back(e, nullptr);
}

template <typename T>
void Dispatcher<T>::SendEvent(const T& e)
{
    DVASSERT_MSG(linkedThreadId != 0, "Before calling SendEvent you must call LinkToCurrentThread");

    uint64 curThreadId = Thread::GetCurrentIdAsInteger();
    if (linkedThreadId == curThreadId)
    {
        // If blocking call is made from the same thread as thread that calls ProcessEvents
        // simply call ProcessEvents
        {
            LockGuard<Mutex> lock(mutex);
            eventQueue.emplace_back(e, nullptr);
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
            eventQueue.emplace_back(e, &signalEventPool[signalEventIndex]);
        }

        DAVA_BEGIN_BLOCKING_CALL(linkedThreadId);

        signalEventPool[signalEventIndex].Wait();
        signalEventBusyFlag[signalEventIndex].clear(std::memory_order_release);
        semaphore.Post(1);

        DAVA_END_BLOCKING_CALL(linkedThreadId);
    }
}

template <typename T>
void Dispatcher<T>::ProcessEvents()
{
    Vector<EventType> readyEvents;
    {
        LockGuard<Mutex> lock(mutex);
        eventQueue.swap(readyEvents);
    }

    for (const EventType& w : readyEvents)
    {
        eventHandler(w.e);
        if (w.signalEvent != nullptr)
        {
            w.signalEvent->Signal();
        }
    }
}

} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
