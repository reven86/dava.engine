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
#include "Concurrency/Thread.h"
#include "Concurrency/LockGuard.h"
#include "Concurrency/UniqueLock.h"
#include "Platform/DeviceInfo.h"

namespace DAVA
{

JobManager::JobManager() 
: mainJobIDCounter(1)
, mainJobLastExecutedID(0)
, workerDoneSem(0)
{
    uint32 cpuCoresCount = DeviceInfo::GetCpuCount();
    workerThreads.reserve(cpuCoresCount);

    for(uint32 i = 0; i < cpuCoresCount; ++i)
    {
        JobThread * thread = new JobThread(&workerQueue, &workerDoneSem);
        workerThreads.push_back(thread);
    }
}

JobManager::~JobManager()
{
    for(uint32 i = 0; i < workerThreads.size(); ++i)
    {
        SafeDelete(workerThreads[i]);
    }

    workerThreads.clear();
}

void JobManager::Update()
{
    bool hasFinishedJobs = false;

    mainQueueMutex.Lock();
    if(!mainJobs.empty())
    {
        // extract all jobs from queue
        while(!mainJobs.empty())
        {
            curMainJob = mainJobs.front();
            mainJobs.pop_front();

            if(curMainJob.type == JOB_MAINBG)
            {
                // TODO:
                // need implementation
                // be careful with job ID, because waiting depends on id order
                // ...

                DVASSERT(false);
            }

            if(curMainJob.invokerThreadId != Thread::Id() && curMainJob.fn != nullptr)
            {
                // unlock queue mutex until function execution finished
                mainQueueMutex.Unlock();
                curMainJob.fn();
                mainJobLastExecutedID = curMainJob.id;
                mainQueueMutex.Lock();
            }

            curMainJob = MainJob();
        }

        hasFinishedJobs = true;
    }
    mainQueueMutex.Unlock();

    // signal that jobs are finished
    if(hasFinishedJobs)
    {
        LockGuard<Mutex> cvguard(mainCVMutex);
        mainCV.NotifyAll();
    }
}

uint32 JobManager::GetWorkersCount() const
{
    return static_cast<uint32>(workerThreads.size());
}

uint32 JobManager::CreateMainJob(const Function<void()>& fn, eMainJobType mainJobType)
{
    uint32 jobID = 0;

    // if we are already in main thread and requested job shouldn't executed lazy
    // perform that job immediately
    if(Thread::IsMainThread() && mainJobType != JOB_MAINLAZY)
    {
        fn();
    }
    else
    {
        // reserve job ID
        jobID = ++mainJobIDCounter;

        // push requested job into queue
        MainJob job;
        job.fn = fn;
        job.invokerThreadId = Thread::GetCurrentId();
        job.type = mainJobType;
        job.id = jobID;

        {
            LockGuard<Mutex> guard(mainQueueMutex);
            mainJobs.push_back(job);
        }
    }

    return jobID;
}

void JobManager::WaitMainJobs(Thread::Id invokerThreadId /* = 0 */)
{
    if(Thread::IsMainThread())
    {
        // if wait was invoked from main-thread 
        // and there are some jobs user is waiting for
        // we should immediately execute them 
        if(HasMainJobs())
        {
            // just run update, it will execute all of main-thread jobs
            Update();

            // assert is something goes wrong
            DVASSERT(!HasMainJobs() && "Job exepected to be executed at this point, but seems it is still in queue");
        }
    }
    else
    {
        // If main thread is locked by WaitWorkerJobs this instruction will unlock
        // main thread, allowing it to perform all scheduled main-thread jobs
        workerDoneSem.Post();

        // Now check if there are some jobs in the queue and wait for them
        UniqueLock<Mutex> lock(mainCVMutex);
        while(HasMainJobs())
        {
            mainCV.Wait(lock);
        }
    }
}

void JobManager::WaitMainJobID(uint32 mainJobID)
{
    if(Thread::IsMainThread())
    {
        // if wait was invoked from main-thread 
        // and there are some jobs user is waiting for
        // we should immediately execute them 
        if(HasMainJobID(mainJobID))
        {
            // just run update, it will execute all of main-thread jobs
            Update();

            // assert is something goes wrong
            DVASSERT(!HasMainJobID(mainJobID) && "Job exepected to be executed at this point, but seems it is still in queue");
        }
    }
    else
    {
        // If main thread is locked by WaitWorkerJobs this instruction will unlock
        // main thread, allowing it to perform all scheduled main-thread jobs
        workerDoneSem.Post();

        // Now check if there are some jobs in the queue and wait for them
        UniqueLock<Mutex> lock(mainCVMutex);
        while(HasMainJobID(mainJobID))
        {
            mainCV.Wait(lock);
        }
    }
}

bool JobManager::HasMainJobs(Thread::Id invokerThreadId /* = 0 */)
{
    bool ret = false;

    // tread id = 0 as current thread id, so we should get it
    if(Thread::Id() == invokerThreadId)
    {
        invokerThreadId = Thread::GetCurrentId();
    }

    {
        LockGuard<Mutex> guard(mainQueueMutex);
        if(curMainJob.invokerThreadId == invokerThreadId)
        {
            ret = true;
        }
        else
        {
            Deque<MainJob>::const_iterator i = mainJobs.begin();
            Deque<MainJob>::const_iterator end = mainJobs.end();
            for(; i != end; ++i)
            {
                if(i->invokerThreadId == invokerThreadId)
                {
                    ret = true;
                    break;
                }
            }
        }
    }

    return ret;
}

bool JobManager::HasMainJobID(uint32 mainJobID)
{
    return (mainJobID > mainJobLastExecutedID);
}

void JobManager::CreateWorkerJob(const Function<void()>& fn)
{
    workerQueue.Push(fn);
    workerQueue.Signal();
}

void JobManager::WaitWorkerJobs()
{
    while(HasWorkerJobs())
    {
        if(Thread::IsMainThread())
        {
            // We want to be able to wait worker jobs, but at the same time
            // allow any worker job execute main job. Potentially this will cause
            // dead lock, but there is a simple solution:
            // 
            // Every time, worker job is trying to execute WaitMainJobs it will 
            // post workerDoneSem semaphore, that will give a chance to execute main jobs
            // in the following Update() call
            //
            Update();
        }

        workerDoneSem.Wait();
    }
}

bool JobManager::HasWorkerJobs()
{
    return !workerQueue.IsEmpty();
}

}
