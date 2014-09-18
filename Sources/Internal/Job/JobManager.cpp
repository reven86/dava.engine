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

JobManager2::JobManager2()
{

}

JobManager2::~JobManager2()
{

}

void JobManager2::Update()
{
    RunMain();
}

void JobManager2::CreateMainJob(const Function<void()>& mainFn, eMainJobType mainJobType)
{
    if(Thread::IsMainThread())
    {
        mainFn();
    }
    else
    {
        mainMutex.Lock();

        MainJob job;
        job.fn = mainFn;
        job.invokerThreadId = Thread::GetCurrentId();
        job.type = mainJobType;
        mainJobs.push_back(job);

        mainMutex.Unlock();
    }
}

void JobManager2::WaitMainJobs(Thread::Id invokerThreadId /* = 0 */)
{
    Mutex mutex;
    mutex.Lock();

    while(HasMainJobs(invokerThreadId))
    {
        Thread::Wait(&mainCV, &mutex);
    }
}

bool JobManager2::HasMainJobs(Thread::Id invokerThreadId /* = 0 */)
{
    bool ret = false;

    if(0 == invokerThreadId)
    {
        invokerThreadId = Thread::GetCurrentId();
    }

    if(invokerThreadId != Thread::GetMainId())
    {
        mainMutex.Lock();

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

        mainMutex.Unlock();
    }

    return ret;
}

void JobManager2::CreateWorkerJob(FastName workerJobTag, const Function<void()>& workerFn)
{

}

void JobManager2::WaitWorkerJobs(FastName workerJobTag)
{

}

bool JobManager2::HasWorkerJobs(FastName workerJobTag)
{
    return false;
}

void JobManager2::RunMain()
{
    // extract current job from queue
    mainMutex.Lock();
    if(!mainJobs.empty())
    {
        // TODO:
        // extract depending on type MAIN or MAINBG
        curMainJob = mainJobs.front();
        mainJobs.pop_front();
    }
    mainMutex.Unlock();

    if(curMainJob.invokerThreadId != 0 && curMainJob.fn != NULL)
    {
        // run job
        curMainJob.fn();
    }

    // current job done, set it to default value
    mainMutex.Lock();
    curMainJob = MainJob();
    mainMutex.Unlock();

    Thread::Signal(&mainCV);
}

void JobManager2::RunWorker()
{

}

}
