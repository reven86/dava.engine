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

#include "Job/JobScheduler.h"
#include "Job/WorkerThread.h"
#include "Thread/LockGuard.h"
#include "Job/Job.h"
#include "Job/JobWaiter.h"

namespace DAVA
{

JobScheduler::JobScheduler(int32 _workerThreadsCount)
:   workerThreadsCount(_workerThreadsCount)
{
    for(int32 i = 0; i < workerThreadsCount; ++i)
    {
        WorkerThread * thread = new WorkerThread(this);
        workerThreads.push_back(thread);
        PushIdleThread(thread);
    }

    taggedJobsCount.resize(MAX_TAG_VALUE);
}

JobScheduler::~JobScheduler()
{
    for(int32 i = 0; i < workerThreadsCount; ++i)
    {
        WorkerThread * wt = workerThreads[i];
        wt->Stop();
        delete wt;
    }
    workerThreads.clear();
}

void JobScheduler::PushJob(Job * job)
{
    int32 tag = job->GetTag();
    DVASSERT(tag >= -1 && tag <= MAX_TAG_VALUE && "tag must be in -1...999 range");
    
    if(tag >= 0)
    {
        AtomicIncrement(taggedJobsCount[tag]);
    }

    jobQueueMutex.Lock();
    jobQueue.push_back(job);
    jobQueueMutex.Unlock();
    
    Schedule();
}

Job * JobScheduler::PopJob()
{
    LockGuard<Mutex> guard(jobQueueMutex);
    
    Job * job = 0;
    if(!jobQueue.empty())
    {
        job = *jobQueue.begin();
        jobQueue.pop_front();
    }
    
    return job;
}

void JobScheduler::PushIdleThread(WorkerThread * thread)
{
    LockGuard<Mutex> guard(idleThreadsMutex);
    idleThreads.push_back(thread);
}

WorkerThread * JobScheduler::PopIdleThread()
{
    LockGuard<Mutex> guard(idleThreadsMutex);
    
    WorkerThread * thread = 0;
    if(!idleThreads.empty())
    {
        thread = *idleThreads.begin();
        idleThreads.pop_front();
    }
    
    return thread;
}

void JobScheduler::Schedule()
{
    LockGuard<Mutex> guard(scheduleMutex);

    WorkerThread * idleThread = PopIdleThread();
    if(idleThread)
    {
        Job * job = PopJob();
        if(job)
        {
            idleThread->SetActiveJob(job);
            idleThread->Wake();
        }
        else
        {
            PushIdleThread(idleThread);
        }
    }
}



void JobScheduler::OnJobCompleted(Job * job)
{
    int32 tag = job->GetTag();
    if(tag >= 0)
    {
        LockGuard<Mutex> guard(waiterMutex);
        AtomicDecrement(taggedJobsCount[tag]);
        if(taggedJobsCount[tag] == 0)
        {
            //notify that all jobs completed
            Map<int32, TaggedWorkerJobsWaiter*>::iterator it = taggedJobsWaiters.find(tag);
            if(taggedJobsWaiters.end() != it)
            {
                Thread::Signal((*it).second->GetConditionalVariable());
            }
        }
    }
}

JobManager::eWaiterRegistrationResult JobScheduler::RegisterWaiter(TaggedWorkerJobsWaiter * waiter)
{
    LockGuard<Mutex> guard(waiterMutex);

    JobManager::eWaiterRegistrationResult result = JobManager::WAITER_WILL_WAIT;

    if(0 == taggedJobsCount[waiter->GetTag()])
    {
        result = JobManager::WAITER_RETURN_IMMEDIATELY;
    }
    else
    {
        taggedJobsWaiters[waiter->GetTag()] = waiter;
    }

    return result;
}

void JobScheduler::UnregisterWaiter(TaggedWorkerJobsWaiter * waiter)
{
    LockGuard<Mutex> guard(waiterMutex);

    int32 tag = waiter->GetTag();
    Map<int32, TaggedWorkerJobsWaiter*>::iterator it = taggedJobsWaiters.find(tag);
    if(taggedJobsWaiters.end() != it)
    {
        taggedJobsWaiters.erase(it);
    }
}

}
