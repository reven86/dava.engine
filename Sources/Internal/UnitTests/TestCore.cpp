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

#include "TestCore.h"

#include "Debug/DVAssert.h"

#include "UnitTests/TestClassFactory.h"
#include "UnitTests/TestClass.h"

namespace DAVA
{
namespace UnitTests
{

    TestCore::TestClassInfo::TestClassInfo(const char* name_, std::unique_ptr<TestClassFactoryBase>&& factory_)
    : name(name_)
    , factory(std::move(factory_))
{}

TestCore::TestClassInfo::TestClassInfo(TestClassInfo&& other)
    : name(std::move(other.name))
    , factory(std::move(other.factory))
{}

TestCore::TestClassInfo::~TestClassInfo() {}

//////////////////////////////////////////////////////////////////////////
TestCore* TestCore::Instance()
{
    static TestCore core;
    return &core;
}

void TestCore::Init(TestStartedCallback testStartedCallback_, TestFinishedCallback testFinishedCallback_, TestFailedCallback testFailedCallback_)
{
    DVASSERT(testStartedCallback_ != 0 && testFinishedCallback_ != 0 && testFailedCallback_ != 0);
    testStartedCallback = testStartedCallback_;
    testFinishedCallback = testFinishedCallback_;
    testFailedCallback = testFailedCallback_;
}

void TestCore::RunOnlyThisTest(const String& testClassName)
{
    DVASSERT(testClassName.empty() || IsTestRegistered(testClassName));
    runOnlyThisTest = testClassName;
}

bool TestCore::HasTests() const
{
    return runOnlyThisTest.empty() ? !testClasses.empty()
                                   : IsTestRegistered(runOnlyThisTest);
}

bool TestCore::ProcessTests(float32 timeElapsed)
{
    const size_t testClassCount = testClasses.size();
    if (curTestClassIndex < testClassCount)
    {
        if (nullptr == curTestClass)
        {
            TestClassInfo& testClasInfo = testClasses[curTestClassIndex];
            curTestClassName = testClasInfo.name;
            if (runOnlyThisTest.empty() || curTestClassName == runOnlyThisTest)
            {
                curTestClass = testClasInfo.factory->CreateTestClass();
                testStartedCallback(curTestClassName);
            }
            else
            {
                curTestClassIndex += 1;
            }
        }

        if (curTestClass != nullptr)
        {
            if (curTestIndex < curTestClass->TestCount())
            {
                if (!testSetUpInvoked)
                {
                    curTestName = curTestClass->TestName(curTestIndex);
                    curTestClass->SetUp(curTestName);
                    testSetUpInvoked = true;
                }

                curTestClass->RunTest(curTestIndex);
                if (curTestClass->TestComplete(curTestName))
                {
                    testSetUpInvoked = false;
                    curTestClass->TearDown(curTestName);
                    curTestIndex += 1;
                }
                else
                {
                    curTestClass->Update(timeElapsed, curTestName);
                }
            }
            else
            {
                testFinishedCallback(curTestClassName);

                SafeDelete(curTestClass);
                curTestIndex = 0;
                curTestClassIndex += 1;
            }
        }
        return true;
    }
    else
    {
        return false;   // No more tests, finish
    }
}

void TestCore::TestFailed(const String& condition, const char* filename, int lineno, const String& userMessage)
{
    testFailedCallback(curTestClassName, curTestName, condition, filename, lineno, userMessage);
}

void TestCore::RegisterTestClass(const char* name, std::unique_ptr<TestClassFactoryBase>&& factory)
{
    testClasses.emplace_back(name, std::move(factory));
}

bool TestCore::IsTestRegistered(const String& testClassName) const
{
    return testClasses.end() != std::find_if(testClasses.begin(), testClasses.end(), [&testClassName](const TestClassInfo& o) -> bool { return o.name == testClassName; });
}

}   // namespace UnitTests
}   // namespace DAVA
