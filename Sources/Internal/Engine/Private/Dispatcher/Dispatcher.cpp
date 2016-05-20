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
    if (!CombineSizeChangedEvents(e))
    {
        events.push_back(e);
    }
}

void Dispatcher::ProcessEvents(Function<void(const DispatcherEvent&)>& handler)
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

bool Dispatcher::CombineSizeChangedEvents(const DispatcherEvent& e)
{
    if (e.type == DispatcherEvent::WINDOW_SIZE_SCALE_CHANGED)
    {
        Window* w = e.window;
        auto it = std::find_if(events.begin(), events.end(), [w](const DispatcherEvent& o) -> bool {
            return o.type == DispatcherEvent::WINDOW_SIZE_SCALE_CHANGED && w == o.window;
        });
        if (it != events.end())
        {
            it->sizeEvent = e.sizeEvent;
            return true;
        }
    }
    return false;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
