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

namespace DAVA
{

#if defined(__DAVAENGINE_WIN32__)

Thread::ThreadId Thread::mainThreadId;
HDC Thread::currentDC = 0;
HGLRC Thread::secondaryContext = 0;

void Thread::InitMainThread()
{
	mainThreadId = GetCurrentThreadId();
}

bool Thread::IsMainThread()
{
	if (mainThreadId.internalTid == 0)
	{
		Logger::Error("Main thread not initialized");
	}
	return (mainThreadId == GetCurrentThreadId());
}

void Thread::SleepThread(uint32 timeMS)
{
    Sleep(timeMS);
}

DWORD WINAPI ThreadFunc(void* param)
{	
	Thread * t = (Thread*)param;
	t->SetThreadId(Thread::GetCurrentThreadId());

	t->state = Thread::STATE_RUNNING;
	t->msg(t);

	t->state = Thread::STATE_ENDED;
	t->Release();
	
	return 0;
}


void Thread::StartWin32()
{
	HANDLE handle;
	handle = CreateThread 
		(
		0, // Security attributes
		0, // Stack size
		ThreadFunc,
		this,
		CREATE_SUSPENDED,
		0);

    threadHandle = handle;
	
	if(!SetThreadPriority(handle, THREAD_PRIORITY_ABOVE_NORMAL))
	{
		Logger::Error("Thread::StartWin32 error %d", (int32)GetLastError());
	}
	ResumeThread(handle);
}

void Thread::YieldThread()
{
    SwitchToThread();
}

Thread::ThreadId Thread::GetCurrentThreadId()
{
	ThreadId ret;
	ret.internalTid = ::GetCurrentThreadId();

	return ret;
}

void Thread::Join()
{
    if (WaitForSingleObject(threadHandle, INFINITE) != WAIT_OBJECT_0)
        DAVA::Logger::Error("Thread::Join() failed in WaitForSingleObject");
}


#endif 

};
