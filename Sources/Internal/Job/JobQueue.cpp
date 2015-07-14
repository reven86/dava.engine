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


#include "Job/JobQueue.h"
#include "Job/JobManager.h"
#include "Concurrency/LockGuard.h"

namespace DAVA
{

JobQueueWorker::JobQueueWorker(uint32 maxCount /* = 1024 */)
    : jobsMaxCount(maxCount)
    , nextPushIndex(0)
    , nextPopIndex(0)
    , processingCount(0)
{
    jobs = new Function<void()>[jobsMaxCount];
}

JobQueueWorker::~JobQueueWorker()
{
    SafeDeleteArray(jobs);
}

void JobQueueWorker::Push(const Function<void()> &fn)
{
    if(fn != nullptr)
    {
        LockGuard<Spinlock> guard(lock);
        if(nextPushIndex == nextPopIndex && 0 == processingCount)
        {
            nextPushIndex = 0;
            nextPopIndex = 0;
        }

        DVASSERT(nextPushIndex < jobsMaxCount);

        jobs[nextPushIndex++] = fn;
        processingCount++;
    }
}

bool JobQueueWorker::PopAndExec()
{
    bool ret = false;
    Function<void()> fn;

    {
        LockGuard<Spinlock> guard(lock);
        if(nextPopIndex < nextPushIndex)
        {
            fn = jobs[nextPopIndex++];
        }
    }

    if(fn != nullptr)
    {
        fn();

        {
            LockGuard<Spinlock> guard(lock);
            DVASSERT(processingCount > 0);
            processingCount--;
        }

        ret = true;
    }

    return ret;
}

bool JobQueueWorker::IsEmpty()
{
    LockGuard<Spinlock> guard(lock);
    return (nextPopIndex == nextPushIndex && 0 == processingCount);
}

void JobQueueWorker::Signal()
{
    LockGuard<Mutex> guard(jobsInQueueMutex);
    jobsInQueueCV.NotifyOne();
}

void JobQueueWorker::Broadcast()
{
    LockGuard<Mutex> guard(jobsInQueueMutex);
    jobsInQueueCV.NotifyAll();
}

void JobQueueWorker::Wait()
{
    UniqueLock<Mutex> lock(jobsInQueueMutex);
    jobsInQueueCV.Wait(lock);
}

}
