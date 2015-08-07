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
#include "Functional/Function.h"
#include "Base/FastName.h"
#include "Concurrency/Atomic.h"
#include "Concurrency/Mutex.h"
#include "Concurrency/Thread.h"
#include "JobQueue.h"
#include "JobThread.h"

namespace DAVA
{

class JobManager : public Singleton<JobManager>
{
public:
	/*! Available types of main-thread job. */
	enum eMainJobType
	{
		JOB_MAIN = 0,       ///< Run only in the main thread. If job is created from the main thread, function will be run immediately.
		JOB_MAINLAZY,		///< Run only in the main thread. If job is created from the main thread, function will be run on next update.
		JOB_MAINBG,         ///< Run in the main or background thread. !!!!!!! TODO: isn't implemented yet
	};

public:
	JobManager();
	virtual ~JobManager();

	/*! This function should be called periodically from the main thread. All main-thread jobs added to the queue
		will be performed inside this function. 
	*/
	void Update();

	/*! Add function to execute in the main-thread.
		\param [in] fn Function to execute.
		\param [in] mainJobType Type of execution. See ::eMainJobType for detailed description.
        \return id Created job id. This id can be used to wait until this job finished.
	*/
    uint32 CreateMainJob(const Function<void ()>& fn, eMainJobType mainJobType = JOB_MAIN);

	/*! Wait for the main-thread jobs, that were added from other thread with the given ID. 
		\param [in] invokerThreadId Thread ID. By default it is 0, which means that current thread ID will be taken.
	*/
    void WaitMainJobs(Thread::Id invokerThreadId = Thread::Id());

    void WaitMainJobID(uint32 mainJobID);

	/*! Check in there are some main-thread jobs in the queue, that were added from thread with the given ID.
		\param [in] invokerThreadId Thread ID. By default it is 0, which means that current thread ID will be taken.
		\return Return true if there are some jobs, otherwise false.
	*/
    bool HasMainJobs(Thread::Id invokerThreadId = Thread::Id());

    bool HasMainJobID(uint32 mainJobID);

	/*! Returns the number of available worker-threads. */
	uint32 GetWorkersCount() const;

	/*! Add function to execute in the worker-thread.
		\param [in] fn Function to execute.
	*/
	void CreateWorkerJob(const Function<void()>& fn);

	/*! Wait until all worker-thread jobs are executed. */
    void WaitWorkerJobs();

	/*!  Check in there are some not executed worker-thread jobs.
		\return Return true if there are some jobs, otherwise false.
	*/
    bool HasWorkerJobs();

protected:
    struct MainJob
    {
        MainJob() : id(0), type(JOB_MAIN), invokerThreadId(Thread::Id()) {}

        uint32 id;
        eMainJobType type;
        Thread::Id invokerThreadId;

        Function<void ()> fn;
    };

    Atomic<uint32> mainJobIDCounter;
    uint32 mainJobLastExecutedID;

    Mutex mainQueueMutex;
    Mutex mainCVMutex;
    Deque<MainJob> mainJobs;
    ConditionVariable mainCV;
    MainJob curMainJob;

    Semaphore workerDoneSem;
    JobQueueWorker workerQueue;
    Vector<JobThread*> workerThreads;
};

}

#endif //__DAVAENGINE_JOB_MANAGER_H__
