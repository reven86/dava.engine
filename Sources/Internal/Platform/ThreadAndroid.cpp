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



#include "Platform/Thread.h"

#if defined(__DAVAENGINE_ANDROID__)
#include "Platform/TemplateAndroid/CorePlatformAndroid.h"

namespace DAVA
{

#include <unistd.h>

#include <pthread.h>


Thread::ThreadId Thread::mainThreadId = 0;
Thread::ThreadId Thread::glThreadId = 0;
Thread::ThreadId Thread::backgroundUpdateThreadId = 0;

void * PthreadMain (void * param)
{
	Thread * t = (Thread*)param;
	t->SetThreadId(Thread::GetCurrentThreadId());
	
	t->state = Thread::STATE_RUNNING;
	t->msg(t);

	t->state = Thread::STATE_ENDED;
	t->Release();

	pthread_exit(0);
}

void Thread::StartAndroid()
{
    pthread_t threadId;
	pthread_create(&threadId, 0, PthreadMain, (void*)this);
}

bool Thread::IsMainThread()
{
	ThreadId threadId = pthread_self();
	return (mainThreadId == threadId || glThreadId == threadId || backgroundUpdateThreadId == threadId);
}

void Thread::InitMainThread()
{
	mainThreadId = GetCurrentThreadId();
}

void Thread::InitGLThread()
{
	glThreadId = GetCurrentThreadId();
}

void Thread::YieldThread()
{
	sched_yield();
}

Thread::ThreadId Thread::GetCurrentThreadId()
{
	ThreadId ret;
	ret.internalTid = pthread_self();

	return ret;
}

void Thread::RegisterBackgroundThread()
{
	DVASSERT(backgroundUpdateThreadId == 0);
	backgroundUpdateThreadId = GetCurrentThreadId();
}

void Thread::UnRegisterBackgroundThread()
{
	backgroundUpdateThreadId = 0;
}

};

#endif //#if defined(__DAVAENGINE_ANDROID__)

