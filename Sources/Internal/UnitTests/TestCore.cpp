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
#include "Utils/Utils.h"

#include "UnitTests/TestClassFactory.h"
#include "UnitTests/TestClass.h"

namespace DAVA
{
namespace UnitTests
{

TestCore::TestClassInfo::TestClassInfo(const char* name_, TestClassFactoryBase* factory_)
    : name(name_)
    , factory(factory_)
{}

TestCore::TestClassInfo::TestClassInfo(TestClassInfo&& other)
    : name(std::move(other.name))
    , runTest(other.runTest)
    , factory(std::move(other.factory))
    , testedClasses(std::move(other.testedClasses))
{}

TestCore::TestClassInfo::~TestClassInfo() {}

//////////////////////////////////////////////////////////////////////////
TestCore* TestCore::Instance()
{
    static TestCore core;
    return &core;
}

void TestCore::Init(TestClassStartedCallback testClassStartedCallback_, TestClassFinishedCallback testClassFinishedCallback_,
                    TestStartedCallback testStartedCallback_, TestFinishedCallback testFinishedCallback_,
                    TestFailedCallback testFailedCallback_, TestClassDisabledCallback testClassDisabledCallback_)
{
    DVASSERT(testClassStartedCallback_ != nullptr && testClassFinishedCallback_ != nullptr && testClassDisabledCallback_ != nullptr);
    DVASSERT(testStartedCallback_ != nullptr && testFinishedCallback_ != nullptr && testFailedCallback_ != nullptr);
    testClassStartedCallback = testClassStartedCallback_;
    testClassFinishedCallback = testClassFinishedCallback_;
    testStartedCallback = testStartedCallback_;
    testFinishedCallback = testFinishedCallback_;
    testFailedCallback = testFailedCallback_;
    testClassDisabledCallback = testClassDisabledCallback_;
}

void TestCore::RunOnlyTheseTestClasses(const String& testClassNames)
{
    DVASSERT(!runLoopInProgress);
    if (!testClassNames.empty())
    {
        Vector<String> testNames;
        Split(testClassNames, ";", testNames);

        // First, disable all tests
        for (TestClassInfo& x : testClasses)
        {
            x.runTest = false;
        }

        // Enable only specified tests
        for (const String& testName : testNames)
        {
            auto iter = std::find_if(testClasses.begin(), testClasses.end(), [&testName](const TestClassInfo& testClassInfo) -> bool {
                return testClassInfo.name == testName;
            });
            DVASSERT(iter != testClasses.end() && "Test classname is not found among registered tests");
            if (iter != testClasses.end())
            {
                iter->runTest = true;
            }
        }
    }
}

void TestCore::DisableTheseTestClasses(const String& testClassNames)
{
    DVASSERT(!runLoopInProgress);
    if (!testClassNames.empty())
    {
        Vector<String> testNames;
        Split(testClassNames, ";", testNames);

        for (const String& testName : testNames)
        {
            auto iter = std::find_if(testClasses.begin(), testClasses.end(), [&testName](const TestClassInfo& testClassInfo) -> bool {
                return testClassInfo.name == testName;
            });
            DVASSERT(iter != testClasses.end() && "Test classname is not found among registered tests");
            if (iter != testClasses.end())
            {
                iter->runTest = false;
            }
        }
    }
}

bool TestCore::HasTestClasses() const
{
    ptrdiff_t n = std::count_if(testClasses.begin(), testClasses.end(), [](const TestClassInfo& info) -> bool { return info.runTest; });
    return n > 0;
}

bool TestCore::ProcessTests(float32 timeElapsed)
{
    runLoopInProgress = true;
    const size_t testClassCount = testClasses.size();
    if (curTestClassIndex < testClassCount)
    {
        if (nullptr == curTestClass)
        {
            TestClassInfo& testClassInfo = testClasses[curTestClassIndex];
            curTestClassName = testClassInfo.name;
            if (testClassInfo.runTest)
            {
                curTestClass = testClassInfo.factory->CreateTestClass();
                testClassStartedCallback(curTestClassName);
            }
            else
            {
                testClassStartedCallback(curTestClassName);
                testClassDisabledCallback(curTestClassName);
                testClassFinishedCallback(curTestClassName);
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
                    testStartedCallback(curTestClassName, curTestName);
                    curTestClass->SetUp(curTestName);
                    testSetUpInvoked = true;
                }

                curTestClass->RunTest(curTestIndex);
                if (curTestClass->TestComplete(curTestName))
                {
                    testSetUpInvoked = false;
                    curTestClass->TearDown(curTestName);
                    testFinishedCallback(curTestClassName, curTestName);
                    curTestIndex += 1;
                }
                else
                {
                    curTestClass->Update(timeElapsed, curTestName);
                }
            }
            else
            {
                testClassFinishedCallback(curTestClassName);

                if (curTestClass->TestCount() > 0)
                {   // Get and save class names which are covered by test only if test class has tests
                    testClasses[curTestClassIndex].testedClasses = curTestClass->ClassesCoveredByTests();
                }

                SafeDelete(curTestClass);
                curTestIndex = 0;
                curTestClassIndex += 1;
            }
        }
        return true;
    }
    else
    {
        runLoopInProgress = false;
        return false;   // No more tests, finish
    }
}

Map<String, Vector<String>> TestCore::GetTestCoverage()
{
    Map<String, Vector<String>> result;
    for (TestClassInfo& x : testClasses)
    {
        if (!x.testedClasses.empty())
        {
            result.emplace(x.name, std::move(x.testedClasses));
        }
        x.runTest = x.runTest;
    }
    return result;
}

void TestCore::TestFailed(const String& condition, const char* filename, int lineno, const String& userMessage)
{
    testFailedCallback(curTestClassName, curTestName, condition, filename, lineno, userMessage);
}

void TestCore::RegisterTestClass(const char* name, TestClassFactoryBase* factory)
{
    testClasses.emplace_back(name, factory);
}

}   // namespace UnitTests
}   // namespace DAVA
