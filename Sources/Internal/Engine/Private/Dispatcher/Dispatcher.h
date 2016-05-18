#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"
#include "Concurrency/Mutex.h"
#include "Functional/Function.h"

#include "Engine/Private/Dispatcher/DispatcherEvent.h"

namespace DAVA
{
namespace Private
{
class Dispatcher final
{
public:
    Dispatcher();
    ~Dispatcher();

    Dispatcher(const Dispatcher&) = delete;
    Dispatcher& operator=(const Dispatcher&) = delete;

    void PostEvent(const DispatcherEvent& e);
    void ProcessEvents(Function<void(const DispatcherEvent&)>& handler);

private:
    Mutex mutex;
    Vector<DispatcherEvent> events;
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
