#ifndef __DAVA_SIGNAL_BASE_H__
#define __DAVA_SIGNAL_BASE_H__

#include <set>

namespace DAVA {

class SignalBase
{
    friend class SlotHolder;

public:
    virtual ~SignalBase() { }
    virtual void Disconnect(SlotHolder *) = 0;
};

class SlotHolder
{
public:
    ~SlotHolder()
    {
        for (auto signal : trackedSignals)
            signal->Disconnect(this);
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
