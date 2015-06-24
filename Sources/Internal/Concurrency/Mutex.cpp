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


#include "Base/Platform.h"
#ifndef USE_CPP11_CONCURRENCY

#include "Concurrency/Mutex.h"
#include "Debug/DVAssert.h"

namespace DAVA
{

Mutex::Mutex()
{
    int ret = pthread_mutex_init(&mutex, nullptr);
    if (ret != 0)
    {
        Logger::Error("Mutex::Mutex() error: %d", ret);
    }
}

RecursiveMutex::RecursiveMutex()
{
    pthread_mutexattr_t attributes;
    pthread_mutexattr_init(&attributes);
    pthread_mutexattr_settype(&attributes, PTHREAD_MUTEX_RECURSIVE);

    int ret = pthread_mutex_init(&mutex, &attributes);
    if (ret != 0)
    {
        Logger::Error("RecursiveMutex::RecursiveMutex() error: %d", ret);
    }
}

MutexBase::~MutexBase()
{
    int ret = pthread_mutex_destroy(&mutex);
    if (ret != 0)
    {
        Logger::Error("Mutex::~Mutex() error: %d", ret);
    }
}

void MutexBase::Lock()
{
    int ret = pthread_mutex_lock(&mutex);
    if (ret != 0)
    {
        Logger::Error("MutexBase::Lock() error: %d", ret);
    }
}

void MutexBase::Unlock()
{
    int ret = pthread_mutex_unlock(&mutex);
    if (ret != 0)
    {
        Logger::Error("MutexBase::Unlock() error: %d", ret);
    }
}

bool MutexBase::TryLock()
{
    return pthread_mutex_trylock(&mutex) == 0;
}

}  // namespace DAVA

#endif  // !USE_CPP11_CONCURRENCY