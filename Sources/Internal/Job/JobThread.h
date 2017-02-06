#ifndef __DAVAENGINE_JOB_THREAD_H__
#define __DAVAENGINE_JOB_THREAD_H__

#include "Concurrency/Semaphore.h"
#include "Concurrency/Thread.h"
#include "JobQueue.h"

namespace DAVA
{
class JobThread
{
public:
    JobThread(JobQueueWorker* workerQueue, Semaphore* workerDoneSem);
    ~JobThread();

    void Cancel();

protected:
    Thread* thread;
    JobQueueWorker* workerQueue;
    Semaphore* workerDoneSem;
    volatile bool threadCancel;
    volatile bool threadFinished;

    void ThreadFunc();
};
}

#endif // __DAVAENGINE_JOB_THREAD_H__
