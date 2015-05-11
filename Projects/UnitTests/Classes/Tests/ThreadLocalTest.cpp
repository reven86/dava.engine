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

#include "Platform/Thread.h"
#include "Thread/ThreadLocal.h"

#include "ThreadLocalTest.h"

using namespace DAVA;

// Sample clas to store in thread local storage
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

ThreadLocal<int32> tlsInt;              // DAVA::int32 fits in pointer size, so store it as is
ThreadLocal<int64*> tlsInt64Ptr;        // DAVA::int64 may or may not fit in pointer size, so store pointer to it
ThreadLocal<TlsClass*> tlsClassPtr;     // Thread local variable can hold only pointers to class

ThreadLocalTest::ThreadLocalTest ()
    : TestTemplate<ThreadLocalTest> ("ThreadLocalTest")
{
    RegisterFunction(this, &ThreadLocalTest::ThreadLocalTestFunc, String("ThreadLocalTestFunc"), nullptr);
}

void ThreadLocalTest::ThreadLocalTestFunc(PerfFuncData * data)
{
    // Set thread local variables in main thread, 
    tlsInt = 1;
    tlsInt64Ptr = new int64(1010);
    tlsClassPtr = new TlsClass(4, "Main thread");

    // Run another thread which changes values of thread local variables
    Thread* thread = Thread::Create(Message(this, &ThreadLocalTest::ThreadFunc));
    thread->Start();
    thread->Join();
    SafeRelease(thread);

    // Make check that main thread's local variables didn't changed by another threads
    TEST_VERIFY(tlsInt == 1);
    TEST_VERIFY(*tlsInt64Ptr == 1010);

    TlsClass* p = tlsClassPtr;
    TEST_VERIFY(p->Int() == 5);
    TEST_VERIFY(p->String() == "Main thread");

    // Delete memory occupied by thread local variables
    delete tlsInt64Ptr;
    tlsInt64Ptr = nullptr;
    delete tlsClassPtr;
    tlsClassPtr = nullptr;
}

void ThreadLocalTest::ThreadFunc(BaseObject*, void*, void*)
{
    // Set thread local variables in another thread
    tlsInt = 333;
    tlsInt64Ptr = new int64(100123);
    tlsClassPtr = new TlsClass(1024, "This is a test");

    // Delete memory occupied by thread local variables
    delete tlsInt64Ptr;
    tlsInt64Ptr = nullptr;
    delete tlsClassPtr;
    tlsClassPtr = nullptr;
}
