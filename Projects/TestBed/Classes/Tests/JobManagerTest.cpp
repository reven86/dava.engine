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

#include <time.h>
#include <string>
#include "JobManagerTest.h"
#include "Job/JobManager.h"
#include "Utils/Random.h"

struct JobManagerTestData
{
    JobManagerTestData() : mainThreadVar(0), testThreadVar(0) { }
    uint32 mainThreadVar;
    uint32 testThreadVar;
};

static void testCalc(uint32 *var)
{
    (*var)++;

    {
        uint32 t = 0;
        uint32 v = *var;

        for(uint32 i = 1; i <= v; ++i)
        {
            t += (i * v) + (t * i);
        }

        *var += ((t & 0x3) - (t & 0x1));
    }
}

JobManagerTest::JobManagerTest()
: TestTemplate<JobManagerTest>("JobManagerTest")
{
    RegisterFunction(this, &JobManagerTest::TestFunction, Format("JobManagerTest"), NULL);
}

void JobManagerTest::LoadResources()
{
}

void JobManagerTest::UnloadResources()
{
}

void JobManagerTest::DidAppear()
{
}

void JobManagerTest::TestFunction(PerfFuncData *data)
{
    TestMainJobs(data);
    TestWorkerJobs(data);
}

void JobManagerTest::TestMainJobs(PerfFuncData *data)
{
    JobManagerTestData testData;

    Thread* thread = Thread::Create(Message(this, &JobManagerTest::ThreadFunc, &testData));
    thread->Start();

    while(thread->GetState() != Thread::STATE_ENDED)
    {
        JobManager::Instance()->Update();
    }

    thread->Join();

    TEST_VERIFY((testData.mainThreadVar == testData.testThreadVar))
}

void JobManagerTest::TestWorkerJobs(PerfFuncData *data)
{
    // TODO:
    // ...
}

void JobManagerTest::ThreadFunc(BaseObject * bo, void * userParam, void * callerParam)
{
    JobManagerTestData *data = (JobManagerTestData*) userParam;

    for(uint32 i = 0; i < 500; i++)
    {
        uint32 count = 50;
        uint32 n = Random::Instance()->Rand(count);
        uint32 jobId = 0;

        for(uint32 j = 0; j < count; ++j)
        {
            // calculate in main thread
            Function<void()> fn = Bind(&testCalc, &data->mainThreadVar);
            uint32 id = JobManager::Instance()->CreateMainJob(fn);

            if(j == n)
            {
                jobId = id;
            }
        }

        JobManager::Instance()->WaitMainJobID(jobId);

        for(uint32 j = 0; j < count; ++j)
        {
            // calculate in this thread
            testCalc(&data->testThreadVar);
        }
    }

    JobManager::Instance()->WaitMainJobs();
}
