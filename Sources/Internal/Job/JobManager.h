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

#ifndef __DAVAENGINE_JOB_MANAGER_H__
#define __DAVAENGINE_JOB_MANAGER_H__

#include "Base/BaseTypes.h"
#include "Base/Singleton.h"
#include "Base/Message.h"
#include "Platform/Thread.h"
#include "Platform/Mutex.h"
#include "Base/ScopedPtr.h"
#include "Job/Job.h"

namespace DAVA
{

class MainThreadJobQueue;
class ThreadIdJobWaiter;
class JobInstanceWaiter;

class JobManager : public Singleton<JobManager>
{
public:
	enum eThreadType
	{
		THREAD_MAIN = 0,
		THREAD_WORKER
	};

	enum eWaiterRegistrationResult
	{
		WAITER_WILL_WAIT,
		WAITER_RETURN_IMMEDIATELY
	};

	JobManager();
	virtual ~JobManager();

	ScopedPtr<Job> CreateJob(eThreadType threadType, const Message & message, uint32 flags = Job::DEFAULT_FLAGS);

	void Update();
	
	void OnJobCreated(Job * job);
	void OnJobCompleted(Job * job);

	eWaiterRegistrationResult RegisterWaiterForCreatorThread(ThreadIdJobWaiter * waiter);
	void UnregisterWaiterForCreatorThread(ThreadIdJobWaiter * waiter);

	eWaiterRegistrationResult RegisterWaiterForJobInstance(JobInstanceWaiter * waiter);
	void UnregisterWaiterForJobInstance(JobInstanceWaiter * waiter);

protected:
	Mutex jobsDoneMutex;
	MainThreadJobQueue * mainQueue;
	void UpdateMainQueue();

	Map<Thread::ThreadId, uint32> jobsPerCreatorThread;
	Map<Thread::ThreadId, ThreadIdJobWaiter *> waitersPerCreatorThread;
	void CheckAndCallWaiterForThreadId(const Thread::ThreadId & threadId);
	
	
	void CheckAndCallWaiterForJobInstance(Job * job);
	Map<Job *, JobInstanceWaiter *> waitersPerJob;
};

}

#endif //__DAVAENGINE_JOB_MANAGER_H__
