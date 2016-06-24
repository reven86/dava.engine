#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"
#include "Concurrency/Mutex.h"
#include "Concurrency/LockGuard.h"
#include "Functional/Function.h"

namespace DAVA
{
namespace Private
{
template <typename T>
class DispatcherT final
{
public:
    DispatcherT() = default;
    ~DispatcherT() = default;

    DispatcherT(const DispatcherT&) = delete;
    DispatcherT& operator=(const DispatcherT&) = delete;

    void PostEvent(const T& e);
    void ProcessEvents(const Function<void(const T&)>& handler);

private:
    Mutex mutex;
    Vector<T> events;
};

template <typename T>
void DispatcherT<T>::PostEvent(const T& e)
{
    LockGuard<Mutex> lock(mutex);
    events.push_back(e);
}

template <typename T>
void DispatcherT<T>::ProcessEvents(const Function<void(const T&)>& handler)
{
    Vector<T> readyEvents;
    {
        LockGuard<Mutex> lock(mutex);
        events.swap(readyEvents);
    }

    for (const T& e : readyEvents)
    {
        handler(e);
    }
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
