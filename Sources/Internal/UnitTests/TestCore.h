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

#ifndef __DAVAENGINE_TESTCORE_H__
#define __DAVAENGINE_TESTCORE_H__

#include "Base/BaseTypes.h"
#include "Functional/Function.h"

namespace DAVA
{
namespace UnitTests
{

class TestClass;
class TestClassFactoryBase;

class TestCore final
{
    struct TestClassInfo
    {
        TestClassInfo(const char* name, TestClassFactoryBase* factory);
        TestClassInfo(TestClassInfo&& other);
        ~TestClassInfo();

        String name;
        bool runTest = true;
        std::unique_ptr<TestClassFactoryBase> factory;
        Vector<String> testedClasses;
    };

public:
    using TestClassStartedCallback = Function<void (const String&)>;
    using TestClassFinishedCallback = Function<void (const String&)>;
    using TestClassDisabledCallback = Function<void (const String&)>;
    using TestStartedCallback = Function<void (const String&, const String&)>;
    using TestFinishedCallback = Function<void (const String&, const String&)>;
    using TestFailedCallback = Function<void (const String&, const String&, const String&, const char*, int, const String&)>;

private:
    TestCore() = default;
    ~TestCore() = default;

public:
    static TestCore* Instance();

    void Init(TestClassStartedCallback testClassStartedCallback, TestClassFinishedCallback testClassFinishedCallback,
              TestStartedCallback testStartedCallback, TestFinishedCallback testFinishedCallback,
              TestFailedCallback testFailedCallback, TestClassDisabledCallback testClassDisabledCallback);

    void RunOnlyTheseTestClasses(const String& testClassNames);
    void DisableTheseTestClasses(const String& testClassNames);

    bool HasTestClasses() const;

    bool ProcessTests(float32 timeElapsed);

    Map<String, Vector<String>> GetTestCoverage();

    void TestFailed(const String& condition, const char* filename, int lineno, const String& userMessage);
    void RegisterTestClass(const char* name, TestClassFactoryBase* factory);

private:
    Vector<TestClassInfo> testClasses;

    TestClass* curTestClass = nullptr;
    String curTestClassName;
    String curTestName;
    size_t curTestClassIndex = 0;
    size_t curTestIndex = 0;
    bool runLoopInProgress = false;
    bool testSetUpInvoked = false;

    TestClassStartedCallback testClassStartedCallback;
    TestClassFinishedCallback testClassFinishedCallback;
    TestClassDisabledCallback testClassDisabledCallback;
    TestStartedCallback testStartedCallback;
    TestFinishedCallback testFinishedCallback;
    TestFailedCallback testFailedCallback;
};

}   // namespace UnitTests
}   // namespace DAVA

#endif  // __DAVAENGINE_TESTCORE_H__
