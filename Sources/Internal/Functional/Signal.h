/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#ifndef __DAVA_SIGNAL_H__
#define __DAVA_SIGNAL_H__

#include "Base/BaseTypes.h"
#include "Concurrency/Mutex.h"
#include "Concurrency/LockGuard.h"
#include "Concurrency/Thread.h"
#include "Concurrency/Atomic.h"
#include "Functional/Function.h"
#include "Functional/SignalBase.h"

// #define ENABLE_MULTITHREADED_SIGNALS // <-- this still isn't implemented

namespace DAVA  {
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
    SigConnectionID Connect(const Fn& fn, ThreadIDType tid = {})
    {
        LockGuard<MutexType> guard(mutex);
        return AddConnection(nullptr, Func(fn), tid);
    }

    template<typename Obj, typename Cls>
    SigConnectionID Connect(Obj *obj, void (Cls::* const& fn)(Args...), ThreadIDType tid = ThreadIDType())
    {
        LockGuard<MutexType> guard(mutex);
        return AddConnection(TrackedObject::Cast(obj), Func(obj, fn), tid);
    }

    template<typename Obj, typename Cls>
    SigConnectionID Connect(Obj *obj, void (Cls::* const& fn)(Args...) const, ThreadIDType tid = ThreadIDType())
    {
        LockGuard<MutexType> guard(mutex);
        return AddConnection(TrackedObject::Cast(obj), Func(obj, fn), tid);
    }

    void Disconnect(SigConnectionID id)
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

    void Disconnect(TrackedObject *obj) override final
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

    void DisconnectAll()
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

    void Track(SigConnectionID id, TrackedObject* obj)
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

    virtual void Emit(Args&&...) const = 0;

protected:
    struct ConnData
    {
        ConnData(Func&& fn_, TrackedObject* obj_, ThreadIDType tid_)
            : fn(std::move(fn_)), obj(obj_), tid(tid_), blocked(false)
        { }

        Func fn;
        TrackedObject* obj;
        ThreadIDType tid;
        bool blocked;
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


template<typename... Args>
class Signal final : public Sig11::SignalImpl<Sig11::DummyMutex, Sig11::DymmyThreadID, Args...>
{
public:
    using Base = Sig11::SignalImpl<Sig11::DummyMutex, Sig11::DymmyThreadID, Args...>;
    
    Signal() = default;
    Signal(const Signal &) = delete;
    Signal& operator=(const Signal &) = delete;

    void Emit(Args&&... args) const override
    {
        for (auto&& con : Base::connections)
        {
            if (!con.second.blocked)
            {
                con.second.fn(std::forward<Args>(args)...);
            }
        }
    }
};

class SignalConnection
{
public:
    SignalConnection(SigConnectionID connId,
                     const Function<void(SigConnectionID)>& unsubscriber)
        : connectionId(connId)
        , unsubscriberFunc(unsubscriber) {}

    ~SignalConnection() 
    {
        Reset();
    }

    void Reset()
    {
        if (unsubscriberFunc)
        {
            unsubscriberFunc(connectionId);
            unsubscriberFunc = nullptr;
        }
    }

    SigConnectionID* Release()
    {
        if (unsubscriberFunc)
        {
            unsubscriberFunc = nullptr;
            return &connectionId;
        }
        return nullptr;
    }

    const SigConnectionID* GetSigConnectionID() const
    {
        return unsubscriberFunc ? &connectionId : nullptr;
    }

private:
    SigConnectionID connectionId;
    Function<void(SigConnectionID)> unsubscriberFunc;
};

template<typename... Args>
SignalConnection MakeSignalConnection(SigConnectionID connId, Signal<Args...>& signal)
{
    using SignalType = Signal<Args...>;
    auto disconnector = [&](SigConnectionID connectionId) { signal.Disconnect(connectionId); };
    return SignalConnection(connId, disconnector);
}

#ifdef ENABLE_MULTITHREADED_SIGNALS

template<typename... Args>
class SignalMt final : public Sig11::SignalImpl<Mutex, Thread::Id, Args...> 
{
    using Base = Sig11::SignalImpl<Mutex, Thread::Id, Args...>;

    SignalMt() = default;
    SignalMt(const SignalMt&) = delete;
    SignalMt& operator=(const SignalMt&) = delete;

    void Emit(Args&&... args) const override
    {
        Thread::Id thisTid = Thread::GetCurrentId();

        LockGuard<Mutex> guard(Base::mutex);
        for (auto&& con : Base::connections)
        {
            if (!con.second.blocked)
            {
                if (con.second.tid == thisTid)
                {
                    con.second.fn(std::forward<Args>(args)...);
                }
                else
                {
                    Function<void()> fn = Bind(con.second.fn, std::forward<Args>(args)...);

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

#endif // __DAVA_SIGNAL_H__
