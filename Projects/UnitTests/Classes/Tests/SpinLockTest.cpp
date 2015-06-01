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

#include "Infrastructure/GameCore.h"
#include "Infrastructure/NewTestFramework.h"

using namespace DAVA;

namespace
{
    uint32 sharedCounter{0};
    const uint32 result{20000000};
    const uint32 numThreads{5};
    Spinlock spin;

    void ThreadFunc(DAVA::BaseObject* obj, void*, void*)
    {
        uint32 count{0};
        while (count < (result / numThreads))
        {
            spin.Lock();
            ++sharedCounter;
            spin.Unlock();
            ++count;
        }
    }
}

DAVA_TESTCLASS(SpinLockTest)
{
    DAVA_TEST(TestFunc)
    {
        static_assert(result % numThreads == 0, "numThreads equal for each thread?");

        List<Thread*> threads;
        for (int i = 0; i < numThreads; ++i)
        {
            threads.push_back(Thread::Create(Message(ThreadFunc)));
        }

        for (auto thread : threads)
        {
            thread->Start();
        }

        for (auto thread : threads)
        {
            thread->Join();
            SafeRelease(thread);
        }

        TEST_VERIFY(result == sharedCounter);
    }
};
