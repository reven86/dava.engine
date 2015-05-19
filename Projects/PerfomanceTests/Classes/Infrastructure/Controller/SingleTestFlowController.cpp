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

#include "SingleTestFlowController.h"

SingleTestFlowController::SingleTestFlowController()
    :   testForRun(nullptr)
    ,   testChooserScreen(nullptr)
    ,   currentScreen(nullptr)

{
}

SingleTestFlowController::SingleTestFlowController(const String& _testName, const BaseTest::TestParams& _testParams)
    :   testForRun(nullptr)
    ,   testChooserScreen(nullptr)
    ,   currentScreen(nullptr)
    ,   testForRunName(_testName)
    ,   testParams(_testParams)

{
}

void SingleTestFlowController::Init(const Vector<BaseTest*>& _testChain)
{
    TestFlowController::Init(_testChain);
    
    if (testForRunName.empty())
    {
        testChooserScreen = new TestChooserScreen(testChain);
        currentScreen = testChooserScreen;
    }
    else
    {
        for (BaseTest* test : _testChain)
        {
            if (test->GetName() == testForRunName)
            {
                testForRun = test;
                testForRun->SetParams(testParams);
                
                if (testParams.frameForDebug > 0 || testParams.targetFrameDelta > 0.001f)
                {
                    testForRun->SetDebuggable(true);
                }
            }
        }

        currentScreen = testForRun;

        if (currentScreen == nullptr)
        {
            Logger::Error(DAVA::Format("Test with name: %s not found", testForRunName.c_str()).c_str());
            Core::Instance()->Quit();
        }
    }
    
}

void SingleTestFlowController::BeginFrame()
{
    if (!currentScreen->IsRegistered())
    {
        currentScreen->RegisterScreen();
        currentScreen->OnStart();
    }
    
    currentScreen->BeginFrame();
}

void SingleTestFlowController::EndFrame()
{ 
    if (nullptr == testForRun)
    {
        if (testChooserScreen->IsFinished())
        {
            testForRun = testChooserScreen->GetTestForRun();
            currentScreen = testForRun;
        }
    }
    else if (testForRun->IsFinished())
    {
        testForRun->OnFinish();

        Logger::Info("Finish all tests.");
        Core::Instance()->Quit();
    }

    currentScreen->EndFrame();
}