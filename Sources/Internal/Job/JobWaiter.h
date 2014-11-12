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

#ifndef __DAVAENGINE_JOB_WAITER_H__
#define __DAVAENGINE_JOB_WAITER_H__

#include "Base/BaseTypes.h"
#include "Platform/Thread.h"

namespace DAVA
{

class Job;

class ThreadIdJobWaiter
{
public:
	ThreadIdJobWaiter(Thread::Id threadId = Thread::GetCurrentId());
	~ThreadIdJobWaiter();
	void Wait();

    inline const Thread::Id &GetThreadId() const;
    inline ConditionalVariable *GetConditionalVariable();
	inline Mutex * GetMutex();

private:
	Thread::Id threadId;
	ConditionalVariable cv;
	Mutex cvmutex;
};

class JobInstanceWaiter
{
public:
	JobInstanceWaiter(Job * job);
	~JobInstanceWaiter();
	void Wait();

    inline ConditionalVariable * GetConditionalVariable();
    inline Job * GetJob() const;
	inline Mutex * GetMutex();

private:
	Job * job;
	ConditionalVariable cv;
	Mutex cvmutex;
};


inline const Thread::Id &ThreadIdJobWaiter::GetThreadId() const
{
    return threadId;
}

inline ConditionalVariable *ThreadIdJobWaiter::GetConditionalVariable()
{
    return &cv;
}

inline Mutex* ThreadIdJobWaiter::GetMutex()
{
	return &cvmutex;
}

inline ConditionalVariable *JobInstanceWaiter::GetConditionalVariable()
{
    return &cv;
}

inline Job *JobInstanceWaiter::GetJob() const
{
    return job;
}

inline Mutex* JobInstanceWaiter::GetMutex()
{
	return &cvmutex;
}

/////
class TaggedWorkerJobsWaiter
{
public:
    TaggedWorkerJobsWaiter(int32 tag);
    ~TaggedWorkerJobsWaiter();
    void Wait();
    ConditionalVariable * GetConditionalVariable();
    int32 GetTag();
    Mutex * GetMutex();

private:
    int32 tag;
    ConditionalVariable cv;
    Mutex mutex;
};

inline ConditionalVariable * TaggedWorkerJobsWaiter::GetConditionalVariable()
{
    return &cv;
}

inline int32 TaggedWorkerJobsWaiter::GetTag()
{
    return tag;
}

inline Mutex * TaggedWorkerJobsWaiter::GetMutex()
{
    return &mutex;
}


}

#endif //__DAVAENGINE_JOB_WAITER_H__