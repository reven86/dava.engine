#ifndef __DAVA_SIGNAL_H__
#define __DAVA_SIGNAL_H__

#include <map>
#include <atomic>

#include "Base/SignalBase.h"
#include "Base/Function.h"
#include "Concurrency/Mutex.h"
#include "Concurrency/LockGuard.h"
#include "Concurrency/Thread.h"

namespace DAVA  {

using SigConnectionID = size_t;
static const SigConnectionID InvalidSigConnectionID = 0;

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
    using Func = Function < void(Args...) > ;

    SignalImpl() = default;
    SignalImpl(const SignalImpl &) = delete;
    SignalImpl& operator=(const SignalImpl &) = delete;

    ~SignalImpl()
    {
        DisconnectAll();
    }

    template<typename Fn>
    SigConnectionID Connect(const Fn& fn, ThreadIDType tid = {}) throw()
    {
        LockGuard<MutexType> guard(mutex);
        return AddConnection(nullptr, Func(fn), tid);
    }

    template<typename Obj>
    SigConnectionID Connect(Obj *obj, void (Obj::* const& fn)(Args...), ThreadIDType tid = ThreadIDType()) throw()
    {
        LockGuard<MutexType> guard(mutex);
        return AddConnection(TrackedObject::Cast(obj), Func(obj, fn), tid);
    }

    template<typename Obj>
    SigConnectionID Connect(Obj *obj, void (Obj::* const& fn)(Args...) const, ThreadIDType tid = ThreadIDType()) throw()
    {
        LockGuard<MutexType> guard(mutex);
        return AddConnection(TrackedObject::Cast(obj), Func(obj, fn), tid);
    }

    void Disconnect(SigConnectionID id) throw()
    {
        LockGuard<MutexType> guard(mutex);

        auto it = connections.find(id);
        if (it != connections.end())
        {
            TrackedObject *obj = it->second.obj;
            if (nullptr != obj)
            {
                obj->Untrack(this);
            }

            connections.erase(it);
        }
    }

    void Disconnect(TrackedObject *obj) throw() override final
    {
        if (nullptr != obj)
        {
            LockGuard<MutexType> guard(mutex);

            auto it = connections.begin();
            auto end = connections.end();

            while (it != end)
            {
                if (it->second.obj == obj)
                {
                    obj->Untrack(this);
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
            TrackedObject *obj = con.second.obj;
            if (nullptr != obj)
            {
                obj->Untrack(this);
            }
        }

        connections.clear();
    }

    void Track(TrackedObject* obj, SigConnectionID id) throw()
    {
        LockGuard<MutexType> guard(mutex);

        auto it = connections.find(id);
        if (it != connections.end())
        {
            if (nullptr != it->second.obj)
            {
                it->second.obj->Untrack(this);
                it->second.obj = nullptr;
            }

            if (nullptr != obj)
            {
                it->second.obj = obj;
                obj->Track(this);
            }
        }
    }

    virtual void Emit(Args&&...) = 0;

protected:
    struct ConnData
    {
        Func fn;
        TrackedObject* obj;
        ThreadIDType tid;
    };

    MutexType mutex;
    std::map<SigConnectionID, ConnData> connections;

    SigConnectionID AddConnection(TrackedObject* obj, Func&& fn, const ThreadIDType& tid) throw()
    {
        static std::atomic<SigConnectionID> counter = {InvalidSigConnectionID};

        SigConnectionID id = ++counter;

        ConnData data;
        data.fn = std::move(fn);
        data.obj = obj;
        data.tid = tid;

        connections.emplace(id, data);

        if (nullptr != obj)
        {
            obj->Track(this);
        }

        return id;
    }
};

} // namespace Sig11


template<typename... Args>
class Signal : public Sig11::SignalImpl<Sig11::DummyMutex, Sig11::DymmyThreadID, Args...>
{
public:
    using Base = Sig11::SignalImpl<Sig11::DummyMutex, Sig11::DymmyThreadID, Args...>;
    
    Signal() = default;
    Signal(const Signal &) = delete;
    Signal& operator=(const Signal &) = delete;

    void Emit(Args&&... args) override
    {
        for (auto&& con : Base::connections)
        {
            con.second.fn(std::forward<Args>(args)...);
        }
    }
};

#if 0
template<typename... Args>
class SignalMt : public Sig11::SignalImpl<Mutex, Thread::Id, Args...>
{
    using Base = Sig11::SignalImpl<Mutex, Thread::Id, Args...>;

    SignalMt() = default;
    SignalMt(const SignalMt&) = delete;
    SignalMt& operator=(const SignalMt&) = delete;

    void Emit(Args&&... args) override
    {
        Thread::Id thisTid = Thread::GetCurrentId();

        LockGuard<Mutex> guard(Base::mutex);
        for (auto&& con : Base::connections)
        {
            if (con.second.tid == thisTid)
            {
                con.second.fn(std::forward<Args>(args)...);
            }
            else
            {
                Function<void()> fn = std::bind(con.second.fn, std::forward<Args>(args)...);

                // TODO:
                // add implementation
                // new to send fn variable directly into thread with given id = con.second.tid
                // ...
            }
        }
    }
};
#endif


} // namespace DAVA

#endif // __DAVA_SIGNAL_H__
