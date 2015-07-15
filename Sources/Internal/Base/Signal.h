#ifndef __DAVA_SIGNAL_H__
#define __DAVA_SIGNAL_H__

#include <map>
#include "SignalBase.h"
#include "Function11.h"

namespace DAVA {

template<typename... Args>
class Signal : public SignalBase
{
public:
    using SlotFn = Function11<void(Args...)>;
    using ConnID = size_t;

    Signal() final = default;
    Signal(const Signal &) = delete;
    Signal(Signal &&) = delete;

    ~Signal()
    {
        DisconnectAll();
    }

    template<typename Fn>
    ConnID Connect(const Fn& fn)
    {
        return AddConnection(nullptr, SlotFn(fn));
    }

    template<typename Obj>
    ConnID Connect(Obj *obj, void (Obj::* const& fn)(Args...))
    {
        return AddConnection(GetHolder(obj), SlotFn(obj, fn));
    }

    template<typename Obj>
    ConnID Connect(Obj *obj, void (Obj::* const& fn)(Args...) const)
    {
        return AddConnection(GetHolder(obj), SlotFn(obj, fn));
    }

    void Disconnect(ConnID id)
    {
        auto it = connections.find(id);
        if (it != connections.end())
        {
            SlotHolder *holder = it->second.holder;
            if (nullptr != holder)
            {
                holder->Untrack(this);
            }

            connections.erase(it);
        }
    }

    void Disconnect(SlotHolder *holder) override final
    {
        if (nullptr != holder)
        {
            auto it = connections.begin();
            auto end = connections.end();

            while (it != end)
            {
                if (it->second.holder == holder)
                {
                    holder->Untrack(this);
                    it = connections.erase(it);
                }
                else
                {
                    it++;
                }
            }
        }
    }

    void DisconnectAll()
    {
        for (auto&& con : connections)
        {
            SlotHolder *holder = con.second.holder;
            if (nullptr != holder)
            {
                holder->Untrack(this);
            }
        }

        connections.clear();
    }

    void Emit(Args&&... args) const
    {
        for (auto&& con : connections)
        {
            con.second.fn(std::forward<Args>(args)...);
        }
    }

    void Track(ConnID id, SlotHolder* holder)
    {
        auto it = connections.find(id);
        if (it != connections.end())
        {
            if (nullptr != it->second.holder)
            {
                it->second.holder->Untrack(this);
                it->second.holder = nullptr;
            }

            if (nullptr != holder)
            {
                it->second.holder = holder;
                holder->Track(this);
            }
        }
    }

protected:
    struct Connection
    {
        SlotFn fn;
        SlotHolder holder;
    };

    std::map<ConnID, Connection> connections;

    ConnID AddConnection(SlotHolder* holder, SlotFn&& fn)
    {
        static atomic<ConnID> counter = 0;

        Connection con;
        con.fn = fn;
        con.holder = holder;

        connections.emplace(++counter, con);

        if (nullptr != holder)
        {
            holder->Track(this);
        }
    }

private:
    template<bool is_base_of_holder = false>
    struct Detail
    {
        SlotHolder* GetHolder(void *t) { return nullptr; }
    };

    template<>
    struct Detail<true>
    {
        template<typename T>
        SlotHolder* GetHolder(T *t) { return static_cast<SlotHolder *>(t); }
    };
};

} // namespace DAVA

#endif // __DAVA_SIGNAL_H__
