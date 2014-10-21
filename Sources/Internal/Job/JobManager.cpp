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

#include "Job/JobManager.h"
#include "Debug/DVAssert.h"
#include "Base/ScopedPtr.h"
#include "Platform/Thread.h"
#include "Thread/LockGuard.h"
#include "Platform/DeviceInfo.h"

namespace DAVA
{

JobManager2::JobManager2()
: workerDoneSem(0)
{
	uint32 cpuCoresCount = DeviceInfo::GetCPUCoresCount();
	workerThreads.reserve(cpuCoresCount);

	for (uint32 i = 0; i < cpuCoresCount; ++i)
	{
		WorkerThread2 * thread = new WorkerThread2(&workerQueue, &workerDoneSem);
		workerThreads.push_back(thread);
	}
}

JobManager2::~JobManager2()
{
	for (uint32 i = 0; i < workerThreads.size(); ++i)
	{
		SafeDelete(workerThreads[i]);
	}

	workerThreads.clear();
}

void JobManager2::Update()
{
    RunMain();
}

uint32 JobManager2::GetWorkersCount() const
{
	return workerThreads.size();
}

void JobManager2::CreateMainJob(const Function<void()>& fn, eMainJobType mainJobType)
{
    if(Thread::IsMainThread() && mainJobType != JOB_MAINLAZY)
    {
        fn();
    }
    else
    {
        LockGuard<Mutex> guard(mainQueueMutex);

        MainJob job;
        job.fn = fn;
        job.invokerThreadId = Thread::GetCurrentId();
        job.type = mainJobType;
        mainJobs.push_back(job);
    }
}

void JobManager2::WaitMainJobs(Thread::Id invokerThreadId /* = 0 */)
{
 	if(Thread::IsMainThread())
 	{
 		RunMain();
 	}
 	else
 	{
		// If main thread is locked by WaitWorkerJobs this instruction will unlock
		// main thread, allowing it to perform all scheduled main-thread jobs
		workerDoneSem.Post();

		LockGuard<Mutex> guard(mainCVMutex);
		while (HasMainJobs(invokerThreadId))
		{
			Thread::Wait(&mainCV, &mainCVMutex);
		}
	}
}

bool JobManager2::HasMainJobs(Thread::Id invokerThreadId /* = 0 */)
{
    bool ret = false;

    if(0 == invokerThreadId)
    {
        invokerThreadId = Thread::GetCurrentId();
    }

    LockGuard<Mutex> guard(mainQueueMutex);

	if(curMainJob.invokerThreadId == invokerThreadId)
    {
        ret = true;
    }
    else
    {
        Deque<MainJob>::iterator i = mainJobs.begin();
        Deque<MainJob>::iterator end = mainJobs.end();
        for(; i != end; ++i)
        {
            if(i->invokerThreadId == invokerThreadId)
            {
                ret = true;
                break;
            }
        }
    }

    return ret;
}

void JobManager2::RunMain()
{
    LockGuard<Mutex> guard(mainQueueMutex);

	if(!mainJobs.empty())
	{
		// TODO:
		// extract depending on type MAIN or MAINBG
		// ...

		// extract all jobs from queue
		while(!mainJobs.empty())
		{
			curMainJob = mainJobs.front();
			mainJobs.pop_front();
        
			if(curMainJob.invokerThreadId != 0 && curMainJob.fn != NULL)
			{
				// unlock queue mutex until function execution finished
				mainQueueMutex.Unlock();
				curMainJob.fn();
				mainQueueMutex.Lock();
			}

			curMainJob = MainJob();
		}

		// signal that jobs are finished
		mainCVMutex.Lock();
		Thread::Broadcast(&mainCV);
		mainCVMutex.Unlock();
	}
}

void JobManager2::CreateWorkerJob(const Function<void()>& fn)
{
	workerQueue.Push(fn);
	workerQueue.Signal();
}

void JobManager2::WaitWorkerJobs()
{
	while(HasWorkerJobs())
	{
		if(Thread::IsMainThread())
		{
			RunMain();
		}

		workerDoneSem.Wait();
	}
}

bool JobManager2::HasWorkerJobs()
{
	return !workerQueue.IsEmpty();
}

JobManager2::WorkerThread2::WorkerThread2(JobQueueWorker *_workerQueue, Semaphore *_workerDoneSem)
: workerQueue(_workerQueue)
, workerDoneSem(_workerDoneSem)
{
	thread = Thread::Create(Message(this, &WorkerThread2::ThreadFunc));
	thread->Start();
}

JobManager2::WorkerThread2::~WorkerThread2()
{
	thread->Cancel();
	workerQueue->Broadcast();
	thread->Join();
	SafeRelease(thread);
}

void JobManager2::WorkerThread2::ThreadFunc(BaseObject * bo, void * userParam, void * callerParam)
{
	while(thread->GetState() != Thread::STATE_CANCELLING)
	{
		workerQueue->Wait();

		while(workerQueue->PopAndExec())
		{ }

		workerDoneSem->Post();
	}
}

}
