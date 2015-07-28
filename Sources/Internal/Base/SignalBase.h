#ifndef __DAVA_SIGNAL_BASE_H__
#define __DAVA_SIGNAL_BASE_H__

#include <set>

namespace DAVA {

class TrackedObject;
    
class SignalBase
{
    friend class TrackedObject;

public:
    virtual ~SignalBase() { }
    virtual void Disconnect(TrackedObject*) = 0;
};

class TrackedObject
{
public:
    ~TrackedObject()
    {
        while (trackedSignals.size() > 0)
        {
            auto it = trackedSignals.begin();
            (*it)->Disconnect(this);
        }
    }

    void Track(SignalBase *signal)
    {
        trackedSignals.insert(signal);
    }

    void Untrack(SignalBase *signal)
    {
        trackedSignals.erase(signal);
    }
    
    template<typename T>
    static TrackedObject* Cast(T *t)
    {
        return Detail<std::is_base_of<TrackedObject, T>::value>::Cast(t);
    }

protected:
    std::set<SignalBase*> trackedSignals;

    template<bool is_derived_from_tracked_obj>
    struct Detail;
};
    
template<>
struct TrackedObject::Detail<false>
{
    static TrackedObject* Cast(void* t) { return nullptr; }
};

template<>
struct TrackedObject::Detail<true>
{
    template<typename T>
    static TrackedObject* Cast(T* t) { return static_cast<TrackedObject *>(t); }
};

} // namespace DAVA

#endif // __DAVA_SIGNAL_BASE_H__
