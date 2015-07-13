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


#ifndef __DAVAENGINE_SEMAPHORE_H__
#define __DAVAENGINE_SEMAPHORE_H__

#include "Base/BaseTypes.h"
#include "Debug/DVAssert.h"

#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__)
#include <dispatch/dispatch.h>
#elif defined(__DAVAENGINE_ANDROID__)
#include <semaphore.h>
#endif //PLATFORMS

namespace DAVA
{

/*! brief Semaphore wrapper class compatible with Thread class. Supports Win32, MacOS, iPhone, Android platforms. */
class Semaphore
{
public:
	Semaphore(uint32 value);
	~Semaphore();

	void Post();
	void Wait();

protected:

#if defined(__DAVAENGINE_WINDOWS__)
	HANDLE semaphore;
#elif defined(__DAVAENGINE_APPLE__)
    dispatch_semaphore_t semaphore;
#elif defined(__DAVAENGINE_ANDROID__)
	sem_t semaphore;
#endif //PLATFORMS

};

#if defined(__DAVAENGINE_WINDOWS__)

// ##########################################################################################################
// Windows implementation
// ##########################################################################################################

inline Semaphore::Semaphore(uint32 value)
{
#ifdef __DAVAENGINE_WIN32__
    semaphore = CreateSemaphore(NULL, value, 0x0FFFFFFF, NULL);
#else
	semaphore = CreateSemaphoreEx(NULL, value, 0x0FFFFFFF, NULL, 0, SEMAPHORE_ALL_ACCESS);
#endif
	DVASSERT(NULL != semaphore);
}

inline Semaphore::~Semaphore()
{
	CloseHandle(semaphore);
}

inline void Semaphore::Post()
{
	ReleaseSemaphore(semaphore, 1, NULL);
}

inline void Semaphore::Wait()
{
	WaitForSingleObjectEx(semaphore, INFINITE, FALSE);
}

#elif defined(__DAVAENGINE_APPLE__) 

// ##########################################################################################################
// MacOS/IOS implementation
// ##########################################################################################################

inline Semaphore::Semaphore(uint32 value)
{
	semaphore = dispatch_semaphore_create(value);
}

inline Semaphore::~Semaphore()
{
	dispatch_release(semaphore);
}

inline void Semaphore::Post()
{
	dispatch_semaphore_signal(semaphore);
}

inline void Semaphore::Wait()
{
	dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);
}

#elif defined(__DAVAENGINE_ANDROID__)

// ##########################################################################################################
// Android implementation
// ##########################################################################################################
inline Semaphore::Semaphore(uint32 value)
{
	sem_init(&semaphore, 0, value);
}

inline Semaphore::~Semaphore()
{
	sem_destroy(&semaphore);
}

inline void Semaphore::Post()
{
	sem_post(&semaphore);
}

inline void Semaphore::Wait()
{
	sem_wait(&semaphore);
}
#endif

};

#endif // __DAVAENGINE_SEMAPHORE_H__