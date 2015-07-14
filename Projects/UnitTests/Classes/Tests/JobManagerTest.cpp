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

#include "DAVAEngine.h"
#include "UnitTests/UnitTests.h"

using namespace DAVA;

#define JOBS_COUNT 500

struct JobManagerTestData
{
    JobManagerTestData() : mainThreadVar(0), testThreadVar(0), ownedMainJobsVar(-1) {}
    uint32 mainThreadVar;
    uint32 testThreadVar;

    int32 ownedMainJobsVar;
};

class TestJobOwner : public BaseObject
{
public:
    TestJobOwner(int32 * _outData) : resultData(_outData), anyData(0) {}
    virtual ~TestJobOwner() { (*resultData) = anyData; };

    void AnyFunction() { anyData++; };

protected:
    int32 * resultData;
    Atomic<int32> anyData;
};

static void testCalc(uint32 *var)
{
    (*var)++;

    {
        uint32 t = 0;
        uint32 v = *var;

        for (uint32 i = 1; i <= v; ++i)
        {
            t += (i * v) + (t * i);
        }

        *var += ((t & 0x3) - (t & 0x1));
    }
}

DAVA_TESTCLASS(JobManagerTest)
{
    DEDUCE_COVERED_CLASS_FROM_TESTCLASS()

    DAVA_TEST(TestMainJobs)
    {
        JobManagerTestData testData;

        Thread* thread = Thread::Create(Message(this, &JobManagerTest::ThreadFunc, &testData));
        thread->Start();

        while (thread->GetState() != Thread::STATE_ENDED)
        {
            JobManager::Instance()->Update();
        }

        thread->Join();

        TEST_VERIFY((testData.mainThreadVar == testData.testThreadVar));
        TEST_VERIFY((testData.ownedMainJobsVar == JOBS_COUNT));
    }

    DAVA_TEST(TestWorkerJobs)
    {
        // TODO:
        // ...
    }

    void ThreadFunc(BaseObject * bo, void * userParam, void * callerParam)
    {
        JobManagerTestData *data = (JobManagerTestData*)userParam;

        for (uint32 i = 0; i < JOBS_COUNT; i++)
        {
            uint32 count = 50;
            uint32 n = Random::Instance()->Rand(count);
            uint32 jobId = 0;

            for (uint32 j = 0; j < count; ++j)
            {
                // calculate in main thread
                Function<void()> fn = std::bind(&testCalc, &data->mainThreadVar);
                uint32 id = JobManager::Instance()->CreateMainJob(fn);

                if (j == n)
                {
                    jobId = id;
                }
            }

            JobManager::Instance()->WaitMainJobID(jobId);

            for (uint32 j = 0; j < count; ++j)
            {
                // calculate in this thread
                testCalc(&data->testThreadVar);
            }
        }

        TestJobOwner * jobOwner = new TestJobOwner(&data->ownedMainJobsVar);
        for (uint32 i = 0; i < JOBS_COUNT; ++i)
        {
            JobManager::Instance()->CreateMainJob(MakeFunction(MakeSharedObject(jobOwner), &TestJobOwner::AnyFunction));
        }
        jobOwner->Release();
        JobManager::Instance()->WaitMainJobs();
    }
};
