#ifndef __DAVA_SIGNAL_BASE_H__
#define __DAVA_SIGNAL_BASE_H__

#include "Base/BaseTypes.h"
#include "Concurrency/Atomic.h"

namespace DAVA
{
using SigConnectionID = size_t;

class TrackedObject;
class SignalBase
{
public:
    virtual ~SignalBase() = default;
    virtual void Disconnect(TrackedObject*) = 0;

    static SigConnectionID GetUniqueConnectionID()
    {
        static Atomic<SigConnectionID> counter = { 0 };
        return ++counter;
    }
};

class TrackedObject
{
public:
    void Track(SignalBase* signal)
    {
        trackedSignals.insert(signal);
    }

    void Untrack(SignalBase* signal)
    {
        trackedSignals.erase(signal);
    }

    template <typename T>
    static TrackedObject* Cast(T* t)
    {
        return Detail<std::is_base_of<TrackedObject, T>::value>::Cast(t);
    }

protected:
    Set<SignalBase*> trackedSignals;

    template <bool is_derived_from_tracked_obj>
    struct Detail;

    virtual ~TrackedObject()
    {
        while (trackedSignals.size() > 0)
        {
            auto it = trackedSignals.begin();
            (*it)->Disconnect(this);
        }
    }
};

template <>
struct TrackedObject::Detail<false>
{
    static TrackedObject* Cast(void* t)
    {
        return nullptr;
    }
};

template <>
struct TrackedObject::Detail<true>
{
    template <typename T>
    static TrackedObject* Cast(T* t)
    {
        return static_cast<TrackedObject*>(t);
    }
};

} // namespace DAVA

#endif // __DAVA_SIGNAL_BASE_H__
