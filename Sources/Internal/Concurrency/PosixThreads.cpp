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


#include "Debug/DVAssert.h"
#include "Concurrency/PosixThreads.h"

#ifdef __DAVAENGINE_WINDOWS__

namespace DAVA
{

int pthread_cond_init(pthread_cond_t *cv, const pthread_condattr_t*)
{
    InitializeConditionVariable(cv);
    return 0;
}

int pthread_cond_destroy(pthread_cond_t* /*cv*/)
{
    return 0;
}

int pthread_cond_wait(pthread_cond_t *cv, pthread_mutex_t *external_mutex)
{
    int res = SleepConditionVariableCS(cv, &external_mutex->critical_section, INFINITE);
    return res != 0 ? 0 : GetLastError();
}

int pthread_cond_signal(pthread_cond_t *cv)
{
    WakeConditionVariable(cv);
    return 0;
}

int pthread_cond_broadcast(pthread_cond_t *cv)
{
    WakeAllConditionVariable(cv);
    return 0;
}

int pthread_mutexattr_init(pthread_mutexattr_t *attr)
{
    attr->isRecursive = false;
    return 0;
}

int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type)
{
    if (type == PTHREAD_MUTEX_RECURSIVE)
        attr->isRecursive = true;
    return 0;
}

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
    BOOL res = InitializeCriticalSectionEx(&mutex->critical_section, 1000, 0);
    if (res == FALSE)
        return GetLastError();

    if (attr != nullptr)
        mutex->attributes = *attr;

    return 0;
}

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
    EnterCriticalSection(&mutex->critical_section);

    //deadlock
    if (!mutex->attributes.isRecursive && 
        mutex->critical_section.RecursionCount > 1)
    {
        DVASSERT_MSG(false, "Thread in deadlocked");
        while (mutex->critical_section.RecursionCount > 1) {}
    }

    return 0;
}

int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
    BOOL res = TryEnterCriticalSection(&mutex->critical_section);
    return res == TRUE ? 0 : EBUSY;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    LeaveCriticalSection(&mutex->critical_section);
    return 0;
}

int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
    DeleteCriticalSection(&mutex->critical_section);
    return 0;
}

};

#endif //__DAVAENGINE_WINDOWS__