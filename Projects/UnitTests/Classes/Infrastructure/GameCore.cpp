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
#include "UnitTests/UnitTests.h"

#include "Infrastructure/GameCore.h"

using namespace DAVA;

namespace
{

String runOnlyThisTest = "";
bool teamcityOutputEnabled = true;

}

void GameCore::OnAppStarted()
{
    if (CommandLineParser::Instance()->CommandIsFound("-only_test"))
    {
        runOnlyThisTest = CommandLineParser::Instance()->GetCommandParam("-only_test");
    }
    if (CommandLineParser::Instance()->CommandIsFound("-noteamcity"))
    {
        teamcityOutputEnabled = false;
    }

    if (teamcityOutputEnabled)
    {
        Logger::Instance()->AddCustomOutput(&teamCityOutput);
    }

    UnitTests::TestCore::Instance()->Init(MakeFunction(this, &GameCore::OnTestStarted),
                                          MakeFunction(this, &GameCore::OnTestFinished),
                                          MakeFunction(this, &GameCore::OnTestFailed));
    if (!runOnlyThisTest.empty())
    {
        UnitTests::TestCore::Instance()->RunOnlyThisTest(runOnlyThisTest);
    }

    if (!UnitTests::TestCore::Instance()->HasTests())
    {
        Logger::Error("%s", "There are no tests.");
        Core::Instance()->Quit();
    }
}

void GameCore::OnAppFinished()
{
    if (teamcityOutputEnabled)
    {
        DAVA::Logger::Instance()->RemoveCustomOutput(&teamCityOutput);
    }
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
void GameCore::OnForeground()
{
    ApplicationCore::OnForeground();
}
#endif //#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)

void GameCore::Update(float32 timeElapsed)
{
    ProcessTests(timeElapsed);
    ApplicationCore::Update(timeElapsed);
}

void GameCore::OnError()
{
    DavaDebugBreak();
}

void GameCore::OnTestStarted(const DAVA::String& testClassName)
{
    Logger::Info("%s", TeamcityTestsOutput::FormatTestStarted(testClassName).c_str());
}

void GameCore::OnTestFinished(const DAVA::String& testClassName)
{
    Logger::Info("%s", TeamcityTestsOutput::FormatTestFinished(testClassName).c_str());
}

void GameCore::OnTestFailed(const String& testClassName, const String& testName, const String& condition, const char* filename, int lineno, const String& userMessage)
{
    OnError();

    String errorString;
    if (userMessage.empty())
    {
        errorString = Format("%s:%d: %s", filename, lineno, testName.c_str());
    }
    else
    {
        errorString = Format("%s:%d: %s (%s)", filename, lineno, testName.c_str(), userMessage.c_str());
    }
    Logger::Error("%s", TeamcityTestsOutput::FormatTestFailed(testClassName, condition, errorString).c_str());
}

void GameCore::ProcessTests(float32 timeElapsed)
{
    if (!UnitTests::TestCore::Instance()->ProcessTests(timeElapsed))
    {
        FinishTests();
    }
}

void GameCore::FinishTests()
{
    if (teamcityOutputEnabled)
    {
        // inform teamcity script we just finished all tests
        // useful on ios devices (run with lldb)
        teamCityOutput.Output(DAVA::Logger::LEVEL_DEBUG, "Finish all tests.");
    }
    else
    {
        Logger::Debug("Finish all tests.");
    }
    Core::Instance()->Quit();
}
