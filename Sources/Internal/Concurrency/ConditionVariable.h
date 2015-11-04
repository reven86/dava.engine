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


#ifndef __DAVAENGINE_CONDITION_VARIABLE_H__
#define __DAVAENGINE_CONDITION_VARIABLE_H__

#include "Base/Platform.h"
#include "Concurrency/Mutex.h"
#include "Concurrency/UniqueLock.h"

#ifdef USE_CPP11_CONCURRENCY
#   include <condition_variable> //for std::condition_variable
#   include "Debug/DVAssert.h"
#else
#   include "Concurrency/PosixThreads.h"
#endif

namespace DAVA
{
    
//-------------------------------------------------------------------------------------------------
//Condition variable class
//-------------------------------------------------------------------------------------------------
class ConditionVariable
{
public:
    ConditionVariable();
    ~ConditionVariable() DAVA_NOEXCEPT;

    ConditionVariable(const ConditionVariable&) = delete;
    ConditionVariable& operator=(const ConditionVariable&) = delete;

    template <typename Predicate>
    void Wait(UniqueLock<Mutex>& guard, Predicate pred);
    void Wait(UniqueLock<Mutex>& guard);

    //mutex must be locked
    template <typename Predicate>
    void Wait(Mutex& mutex, Predicate pred);
    void Wait(Mutex& mutex);

    void NotifyOne();
    void NotifyAll();

private:

#ifdef USE_CPP11_CONCURRENCY
    std::condition_variable cv;
#else
    pthread_cond_t cv;
#endif
};

template <typename Predicate>
void ConditionVariable::Wait(UniqueLock<Mutex>& guard, Predicate pred)
{
    while (!pred())
    {
        Wait(guard);
    }
}

template <typename Predicate>
void ConditionVariable::Wait(Mutex& mutex, Predicate pred)
{
    UniqueLock<Mutex> lock(mutex, AdoptLock());
    Wait(lock, pred);
    lock.Release();
}

inline void ConditionVariable::Wait(Mutex& mutex)
{
    UniqueLock<Mutex> lock(mutex, AdoptLock());
    Wait(lock);
    lock.Release();
}

#ifdef USE_CPP11_CONCURRENCY

//-------------------------------------------------------------------------------------------------
//Condition variable realization using std::condition_variable
//-------------------------------------------------------------------------------------------------
inline ConditionVariable::ConditionVariable() {}
inline ConditionVariable::~ConditionVariable() DAVA_NOEXCEPT{}

inline void ConditionVariable::Wait(UniqueLock<Mutex>& guard)
{
    DVASSERT_MSG(guard.OwnsLock(), "Mutex must be locked and UniqueLock must own it");
    
    std::unique_lock<std::mutex> lock(guard.GetMutex()->mutex, std::adopt_lock_t());
    cv.wait(lock);
    lock.release();
}

inline void ConditionVariable::NotifyOne()
{
    cv.notify_one();
}

inline void ConditionVariable::NotifyAll()
{
    cv.notify_all();
}

#endif //  USE_CPP11_CONCURRENCY

} //  namespace DAVA

#endif //  __DAVAENGINE_CONDITION_VARIABLE_H__