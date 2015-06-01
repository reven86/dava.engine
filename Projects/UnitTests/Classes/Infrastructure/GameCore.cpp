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

#include "Base/BaseTypes.h"
#include "UI/UIScreenManager.h"
#include "TexturePacker/CommandLineParser.h"
#include "Utils/Utils.h"

#include <ctime>

#include "Infrastructure/NewTestFramework.h"
#include "Infrastructure/GameCore.h"

using namespace DAVA;

void GameCore::RunOnlyThisTest()
{
    runOnlyThisTest = "";
}

void GameCore::OnError()
{
    DavaDebugBreak();
}

void GameCore::OnAppStarted()
{
    InitLogging();
    RunOnlyThisTest();

    size_t testStartIndex = 0;
    if (!runOnlyThisTest.empty())
    {
        testStartIndex = Testing::TestClassCollection::Instance()->IsTestClassRegistered(runOnlyThisTest);
    }
    if (0 == Testing::TestClassCollection::Instance()->TestClassCount() || Testing::TestClassCollection::npos == testStartIndex)
    {
        LogMessage(String("There are no tests."));
        Core::Instance()->Quit();
    }
    curTestClassIndex = testStartIndex;
}

void GameCore::OnAppFinished()
{
    DAVA::Logger::Instance()->RemoveCustomOutput(&teamCityOutput);
}

void GameCore::OnSuspend()
{
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    ApplicationCore::OnSuspend();
#endif
}

void GameCore::OnResume()
{
    ApplicationCore::OnResume();
}

#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)
void GameCore::OnBackground()
{
}

void GameCore::OnForeground()
{
    ApplicationCore::OnForeground();
}

void GameCore::OnDeviceLocked()
{

}

#endif //#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)

void GameCore::CreateDocumentsFolder()
{
    FilePath documentsPath = FileSystem::Instance()->GetUserDocumentsPath() + "UnitTests/";
    
    FileSystem::Instance()->CreateDirectory(documentsPath, true);
    FileSystem::Instance()->SetCurrentDocumentsDirectory(documentsPath);
}

File * GameCore::CreateDocumentsFile(const String &filePathname)
{
    FilePath workingFilepathname = FilePath::FilepathInDocuments(filePathname);

    FileSystem::Instance()->CreateDirectory(workingFilepathname.GetDirectory(), true);
    return File::Create(workingFilepathname, File::CREATE | File::WRITE);
}

void GameCore::Update(float32 timeElapsed)
{
    ProcessTests(timeElapsed);
    ApplicationCore::Update(timeElapsed);
}

void GameCore::RegisterError(const String &command, const String &fileName, int32 line, const String& userMessage)
{
    OnError();

    const String& testClassName = Testing::TestClassCollection::Instance()->TestClassName(curTestClassIndex);
    const String& testName = curTestClass->TestName(curTestIndex);
    String errorString;
    if (userMessage.empty())
    {
        errorString = Format("%s:%d: %s", fileName.c_str(), line, testName.c_str());
    }
    else
    {
        errorString = Format("%s:%d: %s (%s)", fileName.c_str(), line, testName.c_str(), userMessage.c_str());
    }
    LogMessage(TeamcityTestsOutput::FormatTestFailed(testClassName, command, errorString));
}

void GameCore::LogMessage(const String &message)
{
    DAVA::Logger::Error(message.c_str());
}

void GameCore::ProcessTests(float32 timeElapsed)
{
    if (nullptr == curTestClass)
    {
        curTestClass = Testing::TestClassCollection::Instance()->CreateTestClass(curTestClassIndex);
        curTestClassName = Testing::TestClassCollection::Instance()->TestClassName(curTestClassIndex);

        Logger::Info(TeamcityTestsOutput::FormatTestStarted(curTestClassName).c_str());
    }

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
        Logger::Info(TeamcityTestsOutput::FormatTestFinished(curTestClassName).c_str());

        SafeDelete(curTestClass);
        curTestIndex = 0;
        curTestClassIndex += 1;

        if (!runOnlyThisTest.empty())
        {
            FinishTests();
        }
    }

    if (curTestClassIndex >= Testing::TestClassCollection::Instance()->TestClassCount())
    {
        FinishTests();
    }
}

void GameCore::FinishTests()
{
    // inform teamcity script we just finished all tests
    // useful on ios devices (run with lldb)
    teamCityOutput.Output(DAVA::Logger::LEVEL_DEBUG, "Finish all tests.");
    Core::Instance()->Quit();
}

DAVA::String GameCore::CreateOutputLogFile()
{
    time_t logStartTime = time(0);
    const String logFileName = Format("Reports/%lld.errorlog", logStartTime);
    File* logFile = CreateDocumentsFile(logFileName);
    DVASSERT(logFile);
    SafeRelease(logFile);

    FilePath workingFilepathname = FilePath::FilepathInDocuments(logFileName);
    return workingFilepathname.GetAbsolutePathname();
}

void GameCore::InitLogging()
{
    if (CommandLineParser::Instance()->CommandIsFound("-only_test"))
    {
        runOnlyThisTest = CommandLineParser::Instance()->GetCommandParam("-only_test");
    }
    Logger::Instance()->AddCustomOutput(&teamCityOutput);
}
