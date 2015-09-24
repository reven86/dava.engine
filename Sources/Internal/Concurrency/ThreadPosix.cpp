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
#if !defined(__DAVAENGINE_WINDOWS__)

#include <time.h>
#include <thread>

#include "Concurrency/PosixThreads.h"
#include "Concurrency/Thread.h"

#if defined (__DAVAENGINE_ANDROID__)
#   include <sys/syscall.h>
#   include <unistd.h>
#   include "Platform/TemplateAndroid/CorePlatformAndroid.h"
#   include "Platform/TemplateAndroid/JniHelpers.h"
#elif defined (__DAVAENGINE_APPLE__)
#   import <Foundation/NSAutoreleasePool.h>
#   include <mach/thread_policy.h>
#endif

namespace DAVA
{
// android have no way to kill a thread, so we will use signal to determine
// if we need to end thread
#if defined (__DAVAENGINE_ANDROID__)
void thread_exit_handler(int sig)
{
	if (SIGRTMIN == sig)
	{
	    JNI::DetachCurrentThreadFromJVM();
		pthread_exit(0);
	}
}
#endif

void Thread::Init()
{
#if defined (__DAVAENGINE_ANDROID__)
    handle = 0;
	struct sigaction cancelThreadAction;
	Memset(&cancelThreadAction, 0, sizeof(cancelThreadAction));
	sigemptyset(&cancelThreadAction.sa_mask);
	cancelThreadAction.sa_flags = 0;
	cancelThreadAction.sa_handler = thread_exit_handler;
	sigaction(SIGRTMIN, &cancelThreadAction, NULL);
#endif
}

void Thread::Shutdown()
{
    DVASSERT(STATE_ENDED == state || STATE_KILLED == state);
    Join();
}

void Thread::KillNative()
{
	uint32 ret = 0;
#if defined (__DAVAENGINE_APPLE__)
    ret = pthread_cancel(handle);
#endif
#if defined (__DAVAENGINE_ANDROID__)
    ret = pthread_kill(handle, SIGRTMIN);
#endif
    if (0 != ret)
    {
        Logger::FrameworkDebug("[Thread::Cancel] cannot kill thread: id = %d, error = %d", Thread::GetCurrentId(), ret);
    }
}

void *PthreadMain(void *param)
{
#if defined(__DAVAENGINE_APPLE__)
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
#endif

#if defined (__DAVAENGINE_ANDROID__)
    JNI::AttachCurrentThreadToJVM();
#endif

    Thread *t = static_cast<Thread *>(param);    
#   if defined (__DAVAENGINE_ANDROID__)
    pthread_setname_np(t->handle, t->name.c_str());
    t->system_handle = gettid();
#   elif defined(__DAVAENGINE_APPLE__)
    pthread_setname_np(t->name.c_str());
#   endif

    Thread::ThreadFunction(param);

#if defined (__DAVAENGINE_ANDROID__)
    JNI::DetachCurrentThreadFromJVM();
#endif

#if defined(__DAVAENGINE_APPLE__)
    [pool release];
#endif
    pthread_exit(0);
}

void Thread::Start()
{
    DVASSERT(STATE_CREATED == state);
    Retain();

    pthread_attr_t attr {};
    pthread_attr_init(&attr);
    if (stackSize != 0)
        pthread_attr_setstacksize(&attr, stackSize);

    pthread_create(&handle, &attr, PthreadMain, (void*)this);
    state = STATE_RUNNING;
    
    pthread_attr_destroy(&attr);
}
    
void Thread::Join()
{
    pthread_join(handle, NULL);
}

Thread::Id Thread::GetCurrentId()
{
    return pthread_self();
}

#if defined(__DAVAENGINE_APPLE__)

bool BindToProcessorApple(pthread_t thread, unsigned proc_n)
{
    thread_affinity_policy_data_t policy = { int(proc_n) };
    thread_port_t mach_thread = pthread_mach_thread_np(thread);
    auto res = thread_policy_set(mach_thread, 
                                 THREAD_AFFINITY_POLICY, 
                                 (thread_policy_t)& policy, 1);
    return res == KERN_SUCCESS;
}

#elif defined(__DAVAENGINE_ANDROID__)

bool BindToProcessorAndroid(pid_t pid, unsigned proc_n)
{
    int mask = 1 << proc_n;
    int res = syscall(__NR_sched_setaffinity, pid, sizeof(mask), &mask);
    return res == 0;
}

#endif

bool Thread::BindToProcessor(unsigned proc_n)
{
    DVASSERT(proc_n < std::thread::hardware_concurrency());
	if (proc_n >= std::thread::hardware_concurrency())
        return false;

#if defined(__DAVAENGINE_APPLE__)
    return BindToProcessorApple(handle, proc_n);
#elif defined(__DAVAENGINE_ANDROID__)
    return BindToProcessorAndroid(system_handle, proc_n);
#else

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(proc_n, &cpuset);

    int error = pthread_setaffinity_np(handle, sizeof(cpuset), &cpuset);
    return error == 0;

#endif
}
    
bool calculatePriority(int priority, int* sched_policy, int* sched_prio)
{
    const int lowestPrio = Thread::PRIORITY_LOW;
    const int highestPrio = Thread::PRIORITY_HIGH;
    
    int prio_min = sched_get_priority_min(*sched_policy);
    int prio_max = sched_get_priority_max(*sched_policy);
    
    if (prio_min == -1 || prio_max == -1)
        return false;
    
    int prio = ((priority - lowestPrio) * (prio_max - prio_min) / highestPrio) + prio_min;
    prio = std::max(prio_min, std::min(prio_max, prio));
    
    *sched_prio = prio;
    return true;
}
    
void Thread::SetPriority(eThreadPriority priority)
{
    DVASSERT(state == STATE_RUNNING);
    if (threadPriority == priority)
        return;
     
    threadPriority = priority;
    int sched_policy = 0;
    sched_param param {};
    
    if (pthread_getschedparam(handle, &sched_policy, &param) != 0)
    {
        Logger::FrameworkDebug("[Thread::SetPriority]: Cannot get schedule parameters");
        return;
    }
    
    int prio = 0;
    if (!calculatePriority(priority, &sched_policy, &prio))
    {
        Logger::FrameworkDebug("[Thread::SetPriority]: Cannot determine scheduler priority range");
        return;
    }
    
    param.sched_priority = prio;
    int status = pthread_setschedparam(handle, sched_policy, &param);

    if (status != 0)
    {
        Logger::FrameworkDebug("[Thread::SetPriority]: Cannot set schedule parameters");
        return;
    }
}

} //  namespace DAVA

#endif
