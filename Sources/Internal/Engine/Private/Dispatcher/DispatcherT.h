#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"
#include "Concurrency/Mutex.h"
#include "Concurrency/LockGuard.h"
#include "Concurrency/Semaphore.h"
#include "Concurrency/AutoResetEvent.h"
#include "Concurrency/Thread.h"
#include "Functional/Function.h"

#include <atomic>

namespace DAVA
{
namespace Private
{
template <typename T>
class DispatcherT final
{
public:
    DispatcherT(const Function<void(const T&)>& handler);
    ~DispatcherT() = default;

    DispatcherT(const DispatcherT&) = delete;
    DispatcherT& operator=(const DispatcherT&) = delete;

    void LinkToCurrentThread();
    uint64 GetLinkedThread() const;

    void PostEvent(const T& e);
    void SendEvent(const T& e);
    void ProcessEvents();

private:
    template <typename T>
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
    using EventType = EventWrapper<T>;

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
DispatcherT<T>::DispatcherT(const Function<void(const T&)>& handler)
    : eventHandler(handler)
    , semaphore(poolSize)
{
    for (std::atomic_flag& f : signalEventBusyFlag)
    {
        f.clear();
    }
}

template <typename T>
void DispatcherT<T>::LinkToCurrentThread()
{
    linkedThreadId = Thread::GetCurrentIdAsInteger();
}

template <typename T>
uint64 DispatcherT<T>::GetLinkedThread() const
{
    return linkedThreadId;
}

template <typename T>
void DispatcherT<T>::PostEvent(const T& e)
{
    LockGuard<Mutex> lock(mutex);
    eventQueue.emplace_back(e, nullptr);
}

template <typename T>
void DispatcherT<T>::SendEvent(const T& e)
{
    DVASSERT(linkedThreadId != 0);

    uint64 curThreadId = Thread::GetCurrentIdAsInteger();
    if (linkedThreadId == curThreadId)
    {
        {
            LockGuard<Mutex> lock(mutex);
            eventQueue.emplace_back(e, nullptr);
        }
        ProcessEvents();
    }
    else
    {
        int signalEventIndex = -1;
        semaphore.Wait();
        for (int i = 0; i < poolSize; ++i)
        {
            if (!signalEventBusyFlag[i].test_and_set(std::memory_order_acquire))
            {
                signalEventIndex = i;
                break;
            }
        }
        DVASSERT(signalEventIndex >= 0);

        {
            LockGuard<Mutex> lock(mutex);
            eventQueue.emplace_back(e, &signalEventPool[signalEventIndex]);
        }

        signalEventPool[signalEventIndex].Wait();
        signalEventBusyFlag[signalEventIndex].clear(std::memory_order_release);
        semaphore.Post(1);
    }
}

template <typename T>
void DispatcherT<T>::ProcessEvents()
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

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
