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


#ifndef __DAVAENGINE_MUTEX_H__
#define __DAVAENGINE_MUTEX_H__

#include "Base/Platform.h"

#if defined(USE_CPP11_CONCURRENCY)
#   include <mutex> //for std::mutex and std::recursive_mutex
#else
#   include "Concurrency/PosixThreads.h"
#endif

namespace DAVA
{

//-----------------------------------------------------------------------------
//Mutex realization
//Direct using of mutex is inadvisable. 
//Use LockGuard or ConcurrentObject instead 
//-----------------------------------------------------------------------------
#if defined(USE_CPP11_CONCURRENCY)

template <typename MutexT>
class MutexBase
{
    friend class ConditionVariable;
public:
    MutexBase() = default;
    MutexBase(const MutexBase&) = delete;
    MutexBase& operator=(const MutexBase&) = delete;

    void Lock() { mutex.lock(); }
    void Unlock() { mutex.unlock(); }
    bool TryLock() { return mutex.try_lock(); }

private:
    MutexT mutex;
};

class Mutex final : public MutexBase<std::mutex>
{
public:
    Mutex() = default;
};

class RecursiveMutex final : public MutexBase<std::recursive_mutex>
{
public:
    RecursiveMutex() = default;
};

#else 

//Base mutex class
class MutexBase
{
    friend class ConditionVariable;
public:
    MutexBase() = default;
    ~MutexBase();

    MutexBase(const MutexBase&) = delete;
    MutexBase& operator=(const MutexBase&) = delete;
	
	void Lock();
    void Unlock();
    bool TryLock();
	
protected:
    pthread_mutex_t mutex;
};

//Specialized mutexes
class Mutex final : public MutexBase
{
public:
	Mutex();
    Mutex(const Mutex&) = delete;
    Mutex& operator=(const Mutex&) = delete;
};

class RecursiveMutex final : public MutexBase
{
public:
    RecursiveMutex();
    RecursiveMutex(const RecursiveMutex&) = delete;
    RecursiveMutex& operator=(const RecursiveMutex&) = delete;
};
	
#endif  // defined(USE_CPP11_CONCURRENCY)

};

#endif  // __DAVAENGINE_MUTEX_H__