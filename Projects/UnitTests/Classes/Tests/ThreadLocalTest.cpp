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

#include "Concurrency/ThreadLocalPtr.h"

using namespace DAVA;

// Sample class to store in thread local storage
class TlsClass
{
public:
    TlsClass(int32 i, const String& s) : intValue(i), strValue(s) {}

    int Int() const { return intValue; }
    const DAVA::String& String() const { return strValue; }

private:
    int32 intValue;
    DAVA::String strValue;
};

ThreadLocalPtr<int32> tlsInt;
ThreadLocalPtr<float> tlsFloat;
ThreadLocalPtr<int64> tlsInt64;
ThreadLocalPtr<TlsClass> tlsClass;

DAVA_TESTCLASS(ThreadLocalTest)
{
    DAVA_TEST(ThreadLocalTestFunc)
    {
        // Set thread local variables in main thread, 
        tlsInt.Reset(new int(1));
        tlsFloat.Reset(new float(4.0f));
        tlsInt64.Reset(new int64(1010));
        tlsClass.Reset(new TlsClass(4, "Main thread"));

        // Run another thread which changes values of thread local variables
        Thread* thread = Thread::Create(Message(this, &ThreadLocalTest::ThreadFunc));
        thread->Start();
        thread->Join();
        SafeRelease(thread);

        // Make check that main thread's local variables didn't changed by another threads
        TEST_VERIFY(*tlsInt == 1);
        TEST_VERIFY(*tlsFloat == 4.0f);
        TEST_VERIFY(*tlsInt64 == 1010);

        TEST_VERIFY(tlsClass->Int() == 4);
        TEST_VERIFY(tlsClass->String() == "Main thread");

        // Delete memory occupied by pointer thread local variables
        tlsInt.Reset();
        tlsFloat.Reset();
        tlsInt64.Reset();
        tlsClass.Reset();
    }

    void ThreadFunc(BaseObject*, void*, void*)
    {
        // Set thread local variables in another thread
        tlsInt.Reset(new int(333));
        tlsFloat.Reset(new float(10.0f));
        tlsInt64.Reset(new int64(100123));
        tlsClass.Reset(new TlsClass(1024, "This is a test"));

        // Delete memory occupied by pointer thread local variables
        tlsInt.Reset();
        tlsFloat.Reset();
        tlsInt64.Reset();
        tlsClass.Reset();
    }
};
