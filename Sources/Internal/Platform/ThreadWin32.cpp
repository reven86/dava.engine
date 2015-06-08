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

#include <windows.h>
const DWORD MS_VC_EXCEPTION=0x406D1388;

#pragma pack(push,8)
using THREADNAME_INFO = struct tagTHREADNAME_INFO
{
    DWORD dwType;       // Must be 0x1000.
    LPCSTR szName;      // Pointer to name (in user addr space).
    DWORD dwThreadID;   // Thread ID
    DWORD dwFlags;      // Reserved for future use, must be zero.
};
#pragma pack(pop)

void Thread::Init()
{
    handle = NULL;
}

void Thread::Shutdown()
{
    DVASSERT(STATE_ENDED == state || STATE_KILLED == state);
    if (handle)
    {
        CloseHandle(handle);
        handle = NULL;
    }
}

void Thread::Start()
{
    Retain();
    DVASSERT(STATE_CREATED == state);
    handle = CreateThread 
        (
        0, // Security attributes
        0, // Stack size
        ThreadFunc,
        this,
        CREATE_SUSPENDED,
        0);

    if(!SetThreadPriority(handle, THREAD_PRIORITY_ABOVE_NORMAL))
    {
        Logger::Error("Thread::StartWin32 error %d", (int32)GetLastError());
    }
    ResumeThread(handle);
}

void Thread::Sleep(uint32 timeMS)
{
    ::Sleep(timeMS);
}

DWORD WINAPI ThreadFunc(void* param)
{	
#if defined(__DAVAENGINE_DEBUG__)
    Thread *t = static_cast<Thread *>(param);

    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = t->name.c_str();
    info.dwThreadID = ::GetCurrentThreadId();
    info.dwFlags = 0;

    __try
    {
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(DWORD), (DWORD*)&info );
    }
    __except(EXCEPTION_CONTINUE_EXECUTION)
    {
    }
#endif

    Thread::ThreadFunction(param);
	return 0;
}

void Thread::Yield()
{
    ::SwitchToThread();
}

void Thread::Join()
{
    if (WaitForSingleObject(handle, INFINITE) != WAIT_OBJECT_0)
    {
        DAVA::Logger::Error("Thread::Join() failed in WaitForSingleObject");
    }
}

void Thread::KillNative()
{
    TerminateThread(handle, 0);
}

Thread::Id Thread::GetCurrentId()
{
    return ::GetCurrentThreadId();
}

#endif 

};
