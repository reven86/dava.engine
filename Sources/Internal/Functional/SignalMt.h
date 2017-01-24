#pragma once

// TODO: should be implemented
// #define ENABLE_MULTITHREADED_SIGNALS

#ifdef ENABLE_MULTITHREADED_SIGNALS
#include "Concurrency/Mutex.h"
#include "Concurrency/Thread.h"
#include "Concurrency/Atomic.h"
#include "Concurrency/LockGuard.h"
#endif

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
