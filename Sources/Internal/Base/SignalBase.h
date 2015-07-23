#ifndef __DAVA_SIGNAL_BASE_H__
#define __DAVA_SIGNAL_BASE_H__

#include <set>

namespace DAVA {

class SignalBase
{
    friend class TrackedObject;

public:
    virtual ~SignalBase() { }
    virtual void Disconnect(TrackedObject *) = 0;
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

protected:
    std::set<SignalBase*> trackedSignals;
};

} // namespace DAVA

#endif // __DAVA_SIGNAL_BASE_H__
