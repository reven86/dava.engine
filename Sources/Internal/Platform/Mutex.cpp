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


#include "Platform/Mutex.h"
#include "FileSystem/Logger.h"

namespace DAVA
{

#if defined(__DAVAENGINE_WIN32__)

Mutex::Mutex()
{
    mutex = CreateMutex(NULL, false, NULL);
    if(mutex == NULL)
    {
        Logger::Error("Mutex::Mutex() error");
    }
}

Mutex::~Mutex()
{
    if(!CloseHandle(mutex))
    {
        Logger::Error("Mutex::~Mutex() error");
    }
}

void Mutex::Lock()
{
    int32 ret = WaitForSingleObject(mutex, INFINITE);
    if(ret == -1)
    {
        Logger::Error("Mutex::Lock() error");
    }
}

void Mutex::Unlock()
{
    if(!ReleaseMutex(mutex))
    {
        return Logger::Error("Mutex::Unlock() error");
    }
}

#elif defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_ANDROID__)

Mutex::Mutex()
{
//	mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_init(&mutex, 0);
}

Mutex::~Mutex()
{
	
}

void Mutex::Lock()
{
	pthread_mutex_lock(&mutex);
}

void Mutex::Unlock()
{
	pthread_mutex_unlock(&mutex);
}

#endif //PLATFORMS


};
