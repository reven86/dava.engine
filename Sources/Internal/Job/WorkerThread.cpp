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

#include "Job/WorkerThread.h"
#include "Job/Job.h"
#include "Job/JobScheduler.h"

namespace DAVA
{

WorkerThread::WorkerThread(JobScheduler * _scheduler)
:   activeJob(0),
    needStop(false),
    scheduler(_scheduler)
{
    thread = Thread::Create(Message(this, &WorkerThread::ThreadFunc));
    thread->Start();
}

WorkerThread::~WorkerThread()
{
    SafeRelease(thread);
}

void WorkerThread::ThreadFunc(BaseObject * bo, void * userParam, void * callerParam)
{
    while(!needStop)
    {
        while(activeJob)
        {
            activeJob->Perform();
            activeJob->SetState(Job::STATUS_DONE);
            scheduler->OnJobCompleted(activeJob);
            activeJob = 0;
            scheduler->PushIdleThread(this);
            scheduler->Schedule();
        }
        Mutex mutex;
        mutex.Lock();
        Thread::Wait(&cv, &mutex);
    }
}

void WorkerThread::Stop()
{
    needStop = true;
}

}
