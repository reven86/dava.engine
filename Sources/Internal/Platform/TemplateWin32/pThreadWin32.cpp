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


#include "Platform/TemplateWin32/pThreadWin32.h"

#ifdef __DAVAENGINE_WIN32__
namespace DAVA
{

int32 pthread_cond_init(pthread_cond_t * cv, const pthread_condattr_t *)
{
    cv->waiters_count_ = 0;
    cv->was_broadcast_ = 0;
    cv->sema_ = CreateSemaphore(NULL,       // no security
        0,          // initially 0
        0x7fffffff, // max count
        NULL);      // unnamed 

    if(cv->sema_ == NULL)
        return GetLastError();

    InitializeCriticalSection(&cv->waiters_count_lock_);
    cv->waiters_done_ = CreateEvent(NULL,  // no security
        FALSE, // auto-reset
        FALSE, // non-signaled initially
        NULL); // unnamed

    if(cv->waiters_done_ == NULL)
        return GetLastError();

    return 0;
}

int pthread_cond_destroy(pthread_cond_t* cv)
{
    DeleteCriticalSection(&cv->waiters_count_lock_);
    if(!CloseHandle(cv->sema_))
        return GetLastError();
    if(!CloseHandle(cv->waiters_done_))
        return GetLastError();
    return 0;
}

int pthread_cond_wait (pthread_cond_t * cv, pthread_mutex_t * external_mutex)
{
    // Avoid race conditions.
    EnterCriticalSection(&cv->waiters_count_lock_);
    cv->waiters_count_++;
    LeaveCriticalSection(&cv->waiters_count_lock_);

    // This call atomically releases the mutex and waits on the
    // semaphore until <pthread_cond_signal> or <pthread_cond_broadcast>
    // are called by another thread.
    SignalObjectAndWait(*external_mutex, cv->sema_, INFINITE, FALSE);

    // Reacquire lock to avoid race conditions.
    EnterCriticalSection(&cv->waiters_count_lock_);

    // We're no longer waiting...
    cv->waiters_count_--;

    // Check to see if we're the last waiter after <pthread_cond_broadcast>.
    int last_waiter = cv->was_broadcast_ && cv->waiters_count_ == 0;

    LeaveCriticalSection(&cv->waiters_count_lock_);

    // If we're the last waiter thread during this particular broadcast
    // then let all the other threads proceed.
    if(last_waiter)
        // This call atomically signals the <waiters_done_> event and waits until
        // it can acquire the <external_mutex>.  This is required to ensure fairness. 
        return SignalObjectAndWait(cv->waiters_done_, *external_mutex, INFINITE, FALSE);
    else
        // Always regain the external mutex since that's the guarantee we
        // give to our callers. 
        return WaitForSingleObject(*external_mutex, INFINITE);
}

int pthread_cond_signal(pthread_cond_t *cv)
{
    EnterCriticalSection(&cv->waiters_count_lock_);
    int have_waiters = cv->waiters_count_ > 0;
    LeaveCriticalSection(&cv->waiters_count_lock_);

    // If there aren't any waiters, then this is a no-op.  
    if (have_waiters)
    {
        if(!ReleaseSemaphore(cv->sema_, 1, 0))
            return GetLastError();
    }

    return 0;
}

int pthread_cond_broadcast(pthread_cond_t *cv)
{
    // This is needed to ensure that <waiters_count_> and <was_broadcast_> are
    // consistent relative to each other.
    EnterCriticalSection(&cv->waiters_count_lock_);
    int have_waiters = 0;

    if(cv->waiters_count_ > 0)
    {
        // We are broadcasting, even if there is just one waiter...
        // Record that we are broadcasting, which helps optimize
        // <pthread_cond_wait> for the non-broadcast case.
        cv->was_broadcast_ = 1;
        have_waiters = 1;
    }

    if(have_waiters)
    {
        // Wake up all the waiters atomically.
        if(!ReleaseSemaphore(cv->sema_, cv->waiters_count_, 0))
            return GetLastError();

        LeaveCriticalSection(&cv->waiters_count_lock_);

        // Wait for all the awakened threads to acquire the counting
        // semaphore. 
        int32 ret = WaitForSingleObject(cv->waiters_done_, INFINITE);
        // This assignment is okay, even without the <waiters_count_lock_> held 
        // because no other waiter threads can wake up to access it.
        cv->was_broadcast_ = 0;

        return ret;
    }
    else
    {
        LeaveCriticalSection(&cv->waiters_count_lock_);
        return 0;
    }
}

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *)
{
    *mutex = CreateMutex(NULL, false, NULL);
    if(*mutex == NULL)
        return GetLastError();
    return 0;
}

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
    int32 ret = WaitForSingleObject(*mutex, INFINITE);
    if(ret == -1)
        return GetLastError();
    return ret;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    if(!ReleaseMutex(*mutex))
        return GetLastError();
    return 0;
}

int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
    if(!CloseHandle(*mutex))
        return GetLastError();
    return 0;
}

};
#endif //__DAVAENGINE_WIN32__