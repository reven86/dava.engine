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
#include "Job/JobQueue.h"
#include "Job/Job.h"
#include "Debug/DVAssert.h"
#include "Base/ScopedPtr.h"
#include "Platform/Thread.h"
#include "Job/JobWaiter.h"
#include "Thread/LockGuard.h"

namespace DAVA
{

JobManager::JobManager()
{
	mainQueue = new MainThreadJobQueue();
}

JobManager::~JobManager()
{
	SafeDelete(mainQueue);
}

void JobManager::Update()
{
	UpdateMainQueue();
}

void JobManager::UpdateMainQueue()
{
	mainQueue->Update();
}

ScopedPtr<Job> JobManager::CreateJob(eThreadType threadType, const Message & message, uint32 flags)
{
	const Thread::Id & creatorThreadId = Thread::GetCurrentId();
	ScopedPtr<Job> job(new Job(message, creatorThreadId, flags));

	if(THREAD_MAIN == threadType)
	{	
		if(Thread::IsMainThread())
		{
			job->SetPerformedOn(Job::PERFORMED_ON_CREATOR_THREAD);
			job->Perform();
            OnJobCompleted(job);
		}
		else
		{
			job->SetPerformedOn(Job::PERFORMED_ON_MAIN_THREAD);
			OnJobCreated(job);
			mainQueue->AddJob(job);
		}
	}
    else if(threadType == THREAD_MAIN_FORCE_ENQUEUE)
    {
        job->SetPerformedOn(Job::PERFORMED_ON_MAIN_THREAD);
        OnJobCreated(job);
        mainQueue->AddJob(job);
    }
	else
	{
		DVASSERT(0);
	}

	return job;
}



void JobManager::OnJobCreated(Job * job)
{
	jobsDoneMutex.Lock();

	jobsPerCreatorThread[job->creatorThreadId]++;

	jobsDoneMutex.Unlock();
}

void JobManager::OnJobCompleted(Job * job)
{
	job->SetState(Job::STATUS_DONE);

	if(Job::PERFORMED_ON_MAIN_THREAD == job->PerformedWhere())
	{
		jobsDoneMutex.Lock();
		//check jobs done for ThreadId
		Map<Thread::Id, uint32>::iterator iter = jobsPerCreatorThread.find(job->creatorThreadId);
		if(iter != jobsPerCreatorThread.end())
		{
			uint32 & jobsCount = (*iter).second;
			DVASSERT(jobsCount> 0);
			jobsCount--;
			if(0 == jobsCount)
			{
				jobsPerCreatorThread.erase(iter);
				CheckAndCallWaiterForThreadId(job->creatorThreadId);
			}

			//check specific job done
			CheckAndCallWaiterForJobInstance(job);
		}

		jobsDoneMutex.Unlock();
	}
}

JobManager::eWaiterRegistrationResult JobManager::RegisterWaiterForCreatorThread(ThreadIdJobWaiter * waiter)
{
	JobManager::eWaiterRegistrationResult result = JobManager::WAITER_RETURN_IMMEDIATELY;
	const Thread::Id threadId = waiter->GetThreadId();

	jobsDoneMutex.Lock();
	//check if all desired jobs are already done
	Map<Thread::Id, uint32>::iterator iter = jobsPerCreatorThread.find(threadId);
	if(iter != jobsPerCreatorThread.end())
	{
		uint32 & jobsCount = (*iter).second;
		if(0 == jobsCount)
		{
			//default value: result = JobManager::WAITER_RETURN_IMMEDIATELY;
		}
		else
		{
			result = JobManager::WAITER_WILL_WAIT;
			waitersPerCreatorThread[threadId] = waiter;
		}
	}

	jobsDoneMutex.Unlock();

	return result;
}

void JobManager::UnregisterWaiterForCreatorThread(ThreadIdJobWaiter * waiter)
{
	jobsDoneMutex.Lock();

	Map<Thread::Id,  ThreadIdJobWaiter *>::iterator it = waitersPerCreatorThread.find(waiter->GetThreadId());
	if(waitersPerCreatorThread.end() != it)
	{
		waitersPerCreatorThread.erase(it);
	}

	jobsDoneMutex.Unlock();
}

void JobManager::CheckAndCallWaiterForThreadId(const Thread::Id & threadId)
{
	Map<Thread::Id,  ThreadIdJobWaiter *>::iterator it = waitersPerCreatorThread.find(threadId);
	if(waitersPerCreatorThread.end() != it)
	{
		Thread::Broadcast(((*it).second)->GetConditionalVariable());
		waitersPerCreatorThread.erase(it);
	}
}

//===================================

JobManager::eWaiterRegistrationResult JobManager::RegisterWaiterForJobInstance(JobInstanceWaiter * waiter)
{
	JobManager::eWaiterRegistrationResult result = JobManager::WAITER_WILL_WAIT;

	Job * job = waiter->GetJob();
	
	if(Job::STATUS_DONE == job->GetState())
	{
		result = JobManager::WAITER_RETURN_IMMEDIATELY;
	}
	else
	{
		jobsDoneMutex.Lock();
		waitersPerJob[job] = waiter;
		jobsDoneMutex.Unlock();
	}

	return result;
}

void JobManager::UnregisterWaiterForJobInstance(JobInstanceWaiter * waiter)
{
	jobsDoneMutex.Lock();

	Map<Job *, JobInstanceWaiter *>::iterator it = waitersPerJob.find(waiter->GetJob());
	if(waitersPerJob.end() != it)
	{
		waitersPerJob.erase(it);
	}

	jobsDoneMutex.Unlock();
}

void JobManager::CheckAndCallWaiterForJobInstance(Job * job)
{
	Map<Job *, JobInstanceWaiter *>::iterator it = waitersPerJob.find(job);
	if(waitersPerJob.end() != it)
	{
		Thread::Broadcast(((*it).second)->GetConditionalVariable());
		waitersPerJob.erase(it);
	}
}

JobManager2::JobManager2(uint32 cpuCoresCount)
{
	workerThreads.reserve(cpuCoresCount);

	for (uint32 i = 0; i < cpuCoresCount; ++i)
	{
		WorkerThread2 * thread = new WorkerThread2(&workerCV);
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
	RunWorker();
}

void JobManager2::CreateMainJob(const Function<void()>& mainFn, eMainJobType mainJobType)
{
    if(Thread::IsMainThread() && mainJobType != JOB_MAINLAZY)
    {
        mainFn();
    }
    else
    {
        LockGuard<Mutex> guard(mainQueueMutex);

        MainJob job;
        job.fn = mainFn;
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

void JobManager2::CreateWorkerJob(FastName workerJobTag, const Function<void()>& workerFn)
{
	workerQueueMutex.Lock();

	WorkerJob job;
	job.fn = workerFn;
	job.tag = workerJobTag;
	workerJobs.push_back(job);

	workerQueueMutex.Unlock();
	RunWorker();
}

void JobManager2::WaitWorkerJobs(FastName workerJobTag)
{
	LockGuard<Mutex> guard(workerCVMutex);
	while(HasWorkerJobs(workerJobTag))
	{
		Thread::Wait(&workerCV, &workerCVMutex);
	}
}

bool JobManager2::HasWorkerJobs(FastName workerJobTag)
{
    bool ret = false;
	LockGuard<Mutex> guard(workerQueueMutex);

	Deque<WorkerJob>::iterator i = workerJobs.begin();
	Deque<WorkerJob>::iterator end = workerJobs.end();
	for(; i != end; ++i)
	{
		if(i->tag == workerJobTag)
		{
			ret = true;
			break;
		}
	}

	if(!ret)
	{
		for(uint32 j = 0; j < workerThreads.size(); ++j)
		{
			if(workerThreads[j]->HasTag(workerJobTag))
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
		Thread::Signal(&mainCV);
		mainCVMutex.Unlock();
	}
}

void JobManager2::RunWorker()
{
	bool next = true;
	LockGuard<Mutex> guard(workerQueueMutex);
	
	if(!workerJobs.empty() && next)
	{
		next = false;

		WorkerJob jobToRun = workerJobs.front();
		for(uint32 i = 0; i < workerThreads.size(); ++i)
		{
			if(workerThreads[i]->Run(jobToRun.tag, jobToRun.fn))
			{
				workerJobs.pop_front();
				next = true;
				break;
			}
		}
	}
}

JobManager2::WorkerThread2::WorkerThread2(ConditionalVariable *_doneCV)
{
	doneCV = _doneCV;

	thread = Thread::Create(Message(this, &WorkerThread2::ThreadFunc));
	thread->Start();
}

JobManager2::WorkerThread2::~WorkerThread2()
{
	thread->Cancel();
	thread->Join();
	SafeRelease(thread);
}

void JobManager2::WorkerThread2::ThreadFunc(BaseObject * bo, void * userParam, void * callerParam)
{
	mutex.Lock();

	while(thread->GetState() != Thread::STATE_CANCELLING)
	{
		Thread::Wait(&cv, &mutex);

		mutex.Unlock();
		fn();
		mutex.Lock();

		fn = NULL;
		tag = FastName();
		Thread::Signal(doneCV);
	}

	mutex.Unlock();
}

bool JobManager2::WorkerThread2::Run(FastName _tag, Function<void()> _fn)
{
	bool ret = false;

	LockGuard<Mutex> guard(mutex);
	if(fn == NULL)
	{
		fn = _fn;
		tag = _tag;
		ret = true;

		Thread::Signal(&cv);
	}

	return ret;
}

bool JobManager2::WorkerThread2::HasTag(FastName _tag)
{
	bool ret = false;

	LockGuard<Mutex> guard(mutex);
	if(tag == _tag)
	{
		ret = true;
	}

	return ret;
}

}
