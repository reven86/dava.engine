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
#include "Base/ScopedPtr.h"
#include "Base/Function.h"
#include "Base/Bind.h"
#include "Base/FastName.h"
#include "Platform/Thread.h"
#include "Platform/Mutex.h"
#include "JobQueue.h"

namespace DAVA
{
class JobManager2 : public Singleton<JobManager2>
{
	friend struct WorkerThread2;

public:
    JobManager2();
    virtual ~JobManager2();

	enum eMainJobType
	{
		JOB_MAIN = 0,       // Run only in the main thread. If job is created from the main thread, function will be run immediately.
		JOB_MAINLAZY,		// Run only in the main thread. If job is created from the main thread, function will be run on next update.
		JOB_MAINBG,         // Run in the main or background thread.
	};

    void Update();

    void CreateMainJob(const Function<void ()>& fn, eMainJobType mainJobType = JOB_MAIN);
    void WaitMainJobs(Thread::Id invokerThreadId = 0);
    bool HasMainJobs(Thread::Id invokerThreadId = 0);

    void CreateWorkerJob(const Function<void ()>& fn);
    void WaitWorkerJobs();
    bool HasWorkerJobs();

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

	struct WorkerThread2
	{
		WorkerThread2(JobQueueWorker *workerQueue, Semaphore *workerDoneSem);
		~WorkerThread2();

		void Cancel();

	protected:
		Thread *thread;
		JobQueueWorker *workerQueue;
		Semaphore *workerDoneSem;

		void ThreadFunc(BaseObject * bo, void * userParam, void * callerParam);
	};

    Mutex mainQueueMutex;
    Mutex mainCVMutex;
    Deque<MainJob> mainJobs;
    ConditionalVariable mainCV;
    MainJob curMainJob;

	Semaphore workerDoneSem;
	JobQueueWorker workerQueue;
	Vector<WorkerThread2*> workerThreads;

    void RunMain();
};

}

#endif //__DAVAENGINE_JOB_MANAGER_H__
