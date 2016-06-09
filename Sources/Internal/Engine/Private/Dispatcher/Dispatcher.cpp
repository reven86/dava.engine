#if defined(__DAVAENGINE_COREV2__)

#include "Concurrency/LockGuard.h"

#include "Engine/Private/Dispatcher/Dispatcher.h"

namespace DAVA
{
namespace Private
{
Dispatcher::Dispatcher() = default;
Dispatcher::~Dispatcher() = default;

void Dispatcher::PostEvent(const DispatcherEvent& e)
{
    LockGuard<Mutex> lock(mutex);
    events.push_back(e);
}

void Dispatcher::ProcessEvents(const Function<void(const DispatcherEvent&)>& handler)
{
    Vector<DispatcherEvent> readyEvents;
    {
        LockGuard<Mutex> lock(mutex);
        events.swap(readyEvents);
    }

    for (const DispatcherEvent& e : readyEvents)
    {
        handler(e);
    }
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
