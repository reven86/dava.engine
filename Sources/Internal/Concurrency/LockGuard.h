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

#ifndef __DAVAENGINE_LOCK_GUARD_H__
#define __DAVAENGINE_LOCK_GUARD_H__

#include "Base/Platform.h"

namespace DAVA
{

// indicates adopt lock
struct AdoptLock {};
// indicates defer lock
struct DeferLock {};
// indicates try to lock
struct TryToLock {};

//-----------------------------------------------------------------------------
//LockGuard class - RAII wrapper for mutex object
//-----------------------------------------------------------------------------
template<class MutexT>
class LockGuard
{
    friend class ConditionVariable;
    using MutexType = MutexT;

public:
    // default construct
    LockGuard() DAVA_NOEXCEPT;

    // construct and lock
    explicit LockGuard(MutexType& mutex);

    // construct and assume already locked
    LockGuard(MutexType& mutex, AdoptLock);

    // construct but don't lock
    LockGuard(MutexType& mutex, DeferLock) DAVA_NOEXCEPT;

    // construct and try to lock
    LockGuard(MutexType& mutex, TryToLock);

    // destructive copy
    LockGuard(LockGuard&& other) DAVA_NOEXCEPT;

    // destructive copy
    LockGuard& operator=(LockGuard&& other) DAVA_NOEXCEPT;

    // clean up
    ~LockGuard() DAVA_NOEXCEPT;

    // no copy construct and assign operator
    LockGuard(const LockGuard&) = delete;
    LockGuard& operator=(const LockGuard&) = delete;

    // lock the mutex
    void Lock();

    // try to lock the mutex
    bool TryLock() DAVA_NOEXCEPT;

    // unlock the mutex
    void Unlock();

    // return true if this object owns the lock
    bool OwnsLock() DAVA_NOEXCEPT;
    
    // release the mutex
    MutexType* Release() DAVA_NOEXCEPT;

private:
    MutexType *mutex_ptr;
    bool owns;
};

//-----------------------------------------------------------------------------
//Native realization of LockGuard
//-----------------------------------------------------------------------------
template<class MutexT>
LockGuard<MutexT>::LockGuard() DAVA_NOEXCEPT
    : mutex_ptr(nullptr), owns(false) {}

template<class MutexT>
LockGuard<MutexT>::LockGuard(MutexType& mutex)
    : mutex_ptr(&mutex), owns(false)
{
    mutex_ptr->Lock();
    owns = true;
}

template<class MutexT>
LockGuard<MutexT>::LockGuard(MutexType& mutex, AdoptLock)
    : mutex_ptr(&mutex), owns(true) {}

template<class MutexT>
LockGuard<MutexT>::LockGuard(MutexType& mutex, DeferLock) DAVA_NOEXCEPT
    : mutex_ptr(&mutex), owns(false) {}

template<class MutexT>
LockGuard<MutexT>::LockGuard(MutexType& mutex, TryToLock)
    : mutex_ptr(&mutex), owns(mutex_ptr->TryLock()) {}

template<class MutexT>
LockGuard<MutexT>::LockGuard(LockGuard&& other) DAVA_NOEXCEPT
    : mutex_ptr(other.mutex_ptr), owns(other.owns)
{
    other.mutex_ptr = 0;
    other.owns = false;
}

template<class MutexT>
LockGuard<MutexT>& LockGuard<MutexT>::operator=(LockGuard&& other) DAVA_NOEXCEPT
{
    if (this != &other)
    {
        if (owns)
            mutex_ptr->Unlock();
        mutex_ptr = other.mutex_ptr;
        owns = other.owns;
        other.mutex_ptr = 0;
        other.owns = false;
    }
    return (*this);
}

template<class MutexT>
LockGuard<MutexT>::~LockGuard() DAVA_NOEXCEPT
{
    if (owns)
    mutex_ptr->Unlock();
}

template<class MutexT>
void LockGuard<MutexT>::Lock()
{
    mutex_ptr->Lock();
    owns = true;
}

template<class MutexT>
bool LockGuard<MutexT>::TryLock() DAVA_NOEXCEPT
{
    owns = mutex_ptr->TryLock();
    return owns;
}

template<class MutexT>
void LockGuard<MutexT>::Unlock()
{
    mutex_ptr->Unlock();
    owns = false;
}

template<class MutexT>
bool LockGuard<MutexT>::OwnsLock() DAVA_NOEXCEPT
{
    return owns;
}

template<class MutexT>
typename LockGuard<MutexT>::MutexType* LockGuard<MutexT>::Release() DAVA_NOEXCEPT
{
    MutexType* mtx = mutex_ptr;
    mutex_ptr = nullptr;
    owns = false;
    return mtx;
}

} //  namespace DAVA

#endif //  __DAVAENGINE_LOCK_GUARD_H__