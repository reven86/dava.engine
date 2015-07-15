#ifndef __DAVA_SIGNAL_H__
#define __DAVA_SIGNAL_H__

#include <map>
#include <atomic>

#include "Base/SignalBase.h"
#include "Base/Function.h"
#include "Concurrency/Mutex.h"
#include "Concurrency/LockGuard.h"

namespace DAVA  {

using SigConnectionID = size_t;

namespace Sig11 {

struct DummyMutex
{
    void Lock() { }
    void Unlock() { }
    bool TryLock() { return true; }
};

struct DymmyThreadID
{ };

template<typename MutexType, typename ThreadIDType, typename... Args>
class SignalImpl : public SignalBase
{
public:
    using Func = Function<void(Args...)>;

    SignalImpl() = default;
    SignalImpl(const SignalImpl &) = delete;
    SignalImpl& operator=(const SignalImpl &) = delete;

    ~SignalImpl()
    {
        DisconnectAll();
    }

    template<typename Fn>
    SigConnectionID Connect(const Fn& fn, ThreadIDType tid = ThreadIDType()) throw()
    {
        LockGuard<MutexType> guard(mutex);
        return AddConnection(nullptr, Func(fn), tid);
    }

    template<typename Obj>
    SigConnectionID Connect(Obj *obj, void (Obj::* const& fn)(Args...), ThreadIDType tid = ThreadIDType()) throw()
    {
        LockGuard<MutexType> guard(mutex);
        return AddConnection(GetHolder(obj), Func(obj, fn), tid);
    }

    template<typename Obj>
    SigConnectionID Connect(Obj *obj, void (Obj::* const& fn)(Args...) const, ThreadIDType tid = ThreadIDType()) throw()
    {
        LockGuard<MutexType> guard(mutex);
        return AddConnection(GetHolder(obj), Func(obj, fn), tid);
    }

    void Disconnect(SigConnectionID id) throw()
    {
        LockGuard<MutexType> guard(mutex);

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

    void Disconnect(SlotHolder *holder) throw() override final
    {
        if (nullptr != holder)
        {
            LockGuard<MutexType> guard(mutex);

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

    void DisconnectAll() throw()
    {
        LockGuard<MutexType> guard(mutex);

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

    void Track(SigConnectionID id, SlotHolder* holder) throw()
    {
        LockGuard<MutexType> guard(mutex);

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

    virtual void Emit(Args&&...) = 0;

protected:
    struct ConnData
    {
        Func fn;
        SlotHolder* holder;
        ThreadIDType tid;
    };

    MutexType mutex;
    std::map<SigConnectionID, ConnData> connections;

    SigConnectionID AddConnection(SlotHolder* holder, Func&& fn, const ThreadIDType& tid) throw()
    {
        static std::atomic<SigConnectionID> counter = 0;

        SigConnectionID id = ++counter;

        ConnData data;
        data.fn = std::move(fn);
        data.holder = holder;
        data.tid = tid;

        connections.emplace(id, data);

        if (nullptr != holder)
        {
            holder->Track(this);
        }

        return id;
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

} // namespace Sig11


template<typename... Args>
class Signal : public Sig11::SignalImpl<Sig11::DummyMutex, Sig11::DymmyThreadID, Args...>
{
public:
    Signal() = default;
    Signal(const Signal &) = delete;
    Signal& operator=(const Signal &) = delete;

    void Emit(Args&&... args) override
    {
        for (auto&& con : connections)
        {
            con.second.fn(std::forward<Args>(args)...);
        }
    }
};

/*
template<typename... Args>
class Signal : public SignalImpl<Args...>
{
public:
    using SlotFn = SignalImpl<Args...>::SlotFn;
    using ConnID = SignalImpl<Args...>::ConnID;

    Signal() final = default;
    Signal(const Signal &) = delete;
    Signal(Signal &&) = delete;

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
        RemoveConnection(id);
    }

    void Disconnect(SlotHolder *holder) override final
    {
        RemoveConnection(holder);
    }

    void DisconnectAll()
    {
        RemoveConnectionAll()
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
        AddTrack(id, holder);
    }
};

template<typename... Args>
class SignalMt : public SignalImpl<Args...>
{
public:
    using SlotFn = SignalImpl<Args...>::SlotFn;
    using ConnID = SignalImpl<Args...>::ConnID;
    using Lock = Mutex;

    SignalMt() final = default;
    SignalMt(const Signal &) = delete;
    SignalMt(Signal &&) = delete;

    template<typename Fn>
    ConnID Connect(const Fn& fn)
    {
        LockGuard<Lock> guard(lock);
        return AddConnection(nullptr, SlotFn(fn));
    }

    template<typename Obj>
    ConnID Connect(Obj *obj, void (Obj::* const& fn)(Args...))
    {
        LockGuard<Lock> guard(lock);
        return AddConnection(GetHolder(obj), SlotFn(obj, fn));
    }

    template<typename Obj>
    ConnID Connect(Obj *obj, void (Obj::* const& fn)(Args...) const)
    {
        LockGuard<Lock> guard(lock);
        return AddConnection(GetHolder(obj), SlotFn(obj, fn));
    }

    void Disconnect(ConnID id)
    {
        LockGuard<Lock> guard(lock);
        RemoveConnection(id);
    }

    void Disconnect(SlotHolder *holder) override final
    {
        LockGuard<Lock> guard(lock);
        RemoveConnection(holder);
    }

    void DisconnectAll()
    {
        LockGuard<Lock> guard(lock);
        RemoveConnectionAll()
    }

    void Emit(Args&&... args) const
    {
        LockGuard<Lock> guard(lock);
        for (auto&& con : connections)
        {
            SlotFn fn = con.second.fn;
            lock.Unlock();
            fn(std::forward<Args>(args)...);
            lock.Lock();
        }
    }

    void Track(ConnID id, SlotHolder* holder)
    {
        LockGuard<Lock> guard(lock);
        AddTrack(id, holder);
    }

protected:
    Lock lock;
};
*/


} // namespace DAVA

#endif // __DAVA_SIGNAL_H__
