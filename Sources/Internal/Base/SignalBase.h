#ifndef __DAVA_SIGNAL_BASE_H__
#define __DAVA_SIGNAL_BASE_H__

#include <set>

namespace DAVA {

class SignalBase;
class SlotHolder
{
public:
    ~SlotHolder()
    {
        for (auto signal : trackedSignals)
        {
            signal->Disconnect(this);
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

class SignalBase
{
    friend class SlotHolder;

public:
    virtual ~SignalBase() { }
    virtual void Disconnect(SlotHolder *) = 0;
};

template<typename... Args>
class SignalImpl : public SignalBase
{
public:
    using SlotFn = Function11<void(Args...)>;
    using ConnID = size_t;

protected:
    struct Connection
    {
        SlotFn fn;
        SlotHolder holder;
    };

    std::map<ConnID, Connection> connections;

protected:


private:
    template<bool is_base_of_holder = false>
    struct Detail
    {
        SlotHolder* GetHolder(void *t) { return nullptr; }
    };

    template<>
    struct Detail < true >
    {
        template<typename T>
        SlotHolder* GetHolder(T *t) { return static_cast<SlotHolder *>(t); }
    };
};

} // namespace DAVA

#endif // __DAVA_SIGNAL_BASE_H__
