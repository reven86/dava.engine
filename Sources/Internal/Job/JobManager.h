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

#include "Base/Function.h"
#include "Base/Bind.h"
#include "Base/FastName.h"

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

	Map<Thread::Id, uint32> jobsPerCreatorThread;
	Map<Thread::Id, ThreadIdJobWaiter *> waitersPerCreatorThread;
	void CheckAndCallWaiterForThreadId(const Thread::Id & threadId);
	
	void CheckAndCallWaiterForJobInstance(Job * job);
	Map<Job *, JobInstanceWaiter *> waitersPerJob;
};

class JobManager2 : public Singleton<JobManager2>
{
public:
    JobManager2();
    virtual ~JobManager2();

	enum eMainJobType
	{
		JOB_MAIN = 0,       // run only in main thread
		JOB_MAINBG,         // run in main or background thread
	};

    void Update();

    void CreateMainJob(const Function<void ()>& mainFn, eMainJobType mainJobType = JOB_MAIN);
    void WaitMainJobs(Thread::Id invokerThreadId = 0);
    bool HasMainJobs(Thread::Id invokerThreadId = 0);

    void CreateWorkerJob(FastName workerJobTag, const Function<void ()>& workerFn);
    void WaitWorkerJobs(FastName workerJobTag);
    bool HasWorkerJobs(FastName workerJobTag);

    // done signal
    // Signal<void (FastName workerJobTag)> workerJobFinished;

protected:
    struct MainJob
    {
        MainJob() : type(JOB_MAIN), invokerThreadId(0) {}

        eMainJobType type;
        Thread::Id invokerThreadId;
        Function<void ()> fn;
    };

    struct WorkerJob
    {
        FastName tag;
        Function<void ()> fn;
    };

    Mutex mainMutex;
    Mutex mainMutexCV;
    Deque<MainJob> mainJobs;
    ConditionalVariable mainCV;
    MainJob curMainJob;

    Mutex workerMutex;
    Mutex workerMutexCV;
    Deque<WorkerJob> workerJobs;
    ConditionalVariable workerCV;

    void RunMain();
    void RunWorker();
};

}

#endif //__DAVAENGINE_JOB_MANAGER_H__
