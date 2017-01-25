#pragma once

// #define ENABLE_MULTITHREADED_SIGNALS // <-- this still isn't implemented

#include "Base/BaseTypes.h"
#include "Functional/Function.h"
#include "Functional/SignalBase.h"

#ifdef ENABLE_MULTITHREADED_SIGNALS
#include "Concurrency/Mutex.h"
#include "Concurrency/Thread.h"
#include "Concurrency/Atomic.h"
#include "Concurrency/LockGuard.h"
#endif

namespace DAVA
{
namespace Sig11
{

#ifdef ENABLE_MULTITHREADED_SIGNALS

template <typename T>
using LockGuard = DAVA::LockGuard<T>;

#else

struct DummyMutex
{
};

template <typename T>
struct DummyLockGuard
{
    DummyLockGuard(const T&)
    {
    }
};

struct DymmyThreadID
{
};

template <typename T>
using LockGuard = DummyLockGuard<T>;

#endif

template <typename MutexType, typename ThreadIDType, typename... Args>
class SignalImpl : public SignalBase
{
public:
    using Func = Function<void(Args...)>;

    SignalImpl() = default;
    SignalImpl(const SignalImpl&) = delete;
    SignalImpl& operator=(const SignalImpl&) = delete;

    ~SignalImpl()
    {
        DisconnectAll();
    }

    template <typename Fn>
    SigConnectionID Connect(const Fn& fn, ThreadIDType tid = {})
    {
        Sig11::LockGuard<MutexType> guard(mutex);
        return AddConnection(nullptr, Func(fn), tid);
    }

    template <typename Obj, typename Fn>
    DAVA_DEPRECATED(SigConnectionID Connect(Obj* obj, const Fn& fn, ThreadIDType tid = {})) //to Smile: it used in case when we need connect static func and use own TrackedObject (see ImGui.cpp)
    {
        Sig11::LockGuard<MutexType> guard(mutex);
        return AddConnection(TrackedObject::Cast(obj), Func(fn), tid);
    }

    template <typename Obj, typename Cls>
    SigConnectionID Connect(Obj* obj, void (Cls::*const& fn)(Args...), ThreadIDType tid = ThreadIDType())
    {
        Sig11::LockGuard<MutexType> guard(mutex);
        return AddConnection(TrackedObject::Cast(obj), Func(obj, fn), tid);
    }

    template <typename Obj, typename Cls>
    SigConnectionID Connect(Obj* obj, void (Cls::*const& fn)(Args...) const, ThreadIDType tid = ThreadIDType())
    {
        Sig11::LockGuard<MutexType> guard(mutex);
        return AddConnection(TrackedObject::Cast(obj), Func(obj, fn), tid);
    }

    void Disconnect(SigConnectionID id)
    {
        Sig11::LockGuard<MutexType> guard(mutex);

        auto it = connections.find(id);
        if (it != connections.end())
        {
            TrackedObject* obj = it->second.obj;
            if (nullptr != obj)
            {
                obj->Untrack(this);
                it->second.obj = nullptr;
            }

            it->second.deleted = true;
        }
    }

    void Disconnect(TrackedObject* obj) override final
    {
        if (nullptr != obj)
        {
            Sig11::LockGuard<MutexType> guard(mutex);

            auto it = connections.begin();
            auto end = connections.end();

            while (it != end)
            {
                if (it->second.obj == obj)
                {
                    obj->Untrack(this);
                    it->second.obj = nullptr;
                    it->second.deleted = true;
                }

                it++;
            }
        }
    }

    void DisconnectAll()
    {
        Sig11::LockGuard<MutexType> guard(mutex);

        for (auto&& con : connections)
        {
            TrackedObject* obj = con.second.obj;
            if (nullptr != obj)
            {
                obj->Untrack(this);
                con.second.obj = nullptr;
            }
            con.second.deleted = true;
        }
    }

    void Track(SigConnectionID id, TrackedObject* obj)
    {
        Sig11::LockGuard<MutexType> guard(mutex);

        auto it = connections.find(id);
        if (it != connections.end())
        {
            if (nullptr != it->second.obj)
            {
                it->second.obj->Untrack(this);
                it->second.obj = nullptr;
            }

            if (nullptr != obj && !it->second.deleted)
            {
                it->second.obj = obj;
                obj->Track(this);
            }
        }
    }

    TrackedObject* GetTracked(SigConnectionID id) const
    {
        TrackedObject* ret = nullptr;

        auto it = connections.find(id);
        if (it != connections.end())
        {
            ret = it->second.obj;
        }

        return ret;
    }

    void Block(SigConnectionID id, bool block)
    {
        auto it = connections.find(id);
        if (it != connections.end())
        {
            it->second.blocked = block;
        }
    }

    bool IsBlocked(SigConnectionID id) const
    {
        bool ret = false;

        auto it = connections.find(id);
        if (it != connections.end())
        {
            ret = it->second.blocked;
        }

        return ret;
    }

    virtual void Emit(Args... args) = 0;

protected:
    struct ConnData
    {
        ConnData(Func&& fn_, TrackedObject* obj_, ThreadIDType tid_)
            : fn(std::move(fn_))
            , obj(obj_)
            , tid(tid_)
            , blocked(false)
            , deleted(false)
        {
        }

        Func fn;
        TrackedObject* obj;
        ThreadIDType tid;
        bool blocked;
        bool deleted;
    };

    MutexType mutex;
    Map<SigConnectionID, ConnData> connections;

private:
    SigConnectionID AddConnection(TrackedObject* obj, Func&& fn, const ThreadIDType& tid)
    {
        SigConnectionID id = SignalBase::GetUniqueConnectionID();
        connections.emplace(std::make_pair(id, ConnData(std::move(fn), obj, tid)));

        if (nullptr != obj)
        {
            obj->Track(this);
        }

        return id;
    }
};

} // namespace Sig11

template <typename... Args>
class Signal final : public Sig11::SignalImpl<Sig11::DummyMutex, Sig11::DymmyThreadID, Args...>
{
public:
    using Base = Sig11::SignalImpl<Sig11::DummyMutex, Sig11::DymmyThreadID, Args...>;

    Signal() = default;
    Signal(const Signal&) = delete;
    Signal& operator=(const Signal&) = delete;

    void Emit(Args... args) override
    {
        auto iter = Base::connections.begin();
        while (iter != Base::connections.end())
        {
            if (iter->second.deleted)
            {
                iter = Base::connections.erase(iter);
            }
            else
            {
                if (!iter->second.blocked)
                {
                    // Make functor copy and call its copy:
                    //  when connected lambda with captured variables disconnects from signal while signal is emitting
                    //  compiler destroys lambda and its captured variables
                    auto fn = iter->second.fn;
                    fn(args...);
                }

                iter++;
            }
        }
    }
};

#ifdef ENABLE_MULTITHREADED_SIGNALS

template <typename... Args>
class SignalMt final : public Sig11::SignalImpl<Mutex, Thread::Id, Args...>
{
    using Base = Sig11::SignalImpl<Mutex, Thread::Id, Args...>;

    SignalMt() = default;
    SignalMt(const SignalMt&) = delete;
    SignalMt& operator=(const SignalMt&) = delete;

    void Emit(Args... args) override
    {
        Thread::Id thisTid = Thread::GetCurrentId();

        Sig11::LockGuard<Mutex> guard(Base::mutex);
        for (auto&& con : Base::connections)
        {
            if (!con.second.blocked)
            {
                if (con.second.tid == thisTid)
                {
                    con.second.fn(args...);
                }
                else
                {
                    Function<void()> fn = Bind(con.second.fn, args...);

                    // TODO:
                    // add implementation
                    // new to send fn variable directly into thread with given id = con.second.tid
                    // ...
                }
            }
        }
    }
};

#endif

} // namespace DAVA
