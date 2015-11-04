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

#include "TestChainFlowController.h"

TestChainFlowController::TestChainFlowController(bool _showUI)
    :   reportScreen(new ReportScreen())
    ,   currentScreen(nullptr)
    ,   currentTest(nullptr)
    ,   currentTestIndex(0)
    ,   showUI(_showUI)
    ,   testsFinished(false)
    
{
}

void TestChainFlowController::Init(const Vector<BaseTest*>& _testChain)
{
    TestFlowController::Init(_testChain);
    
    currentTestIndex = 0;
    currentTest = testChain[currentTestIndex];
    currentTest->ShowUI(showUI);
    currentScreen = currentTest;
}

void TestChainFlowController::BeginFrame()
{
    if (!currentScreen->IsRegistered())
    {   
        currentScreen->RegisterScreen();
        currentScreen->OnStart();
    }
    
    currentScreen->BeginFrame();
}

void TestChainFlowController::EndFrame()
{
    currentScreen->EndFrame();
    
    if (!testsFinished)
    {
        if (currentTest->IsFinished())
        {
            currentTest->OnFinish();
            currentTestIndex++;
            
            testsFinished = testChain.size() == currentTestIndex;
            
            if (!testsFinished)
            {
                currentTest = testChain[currentTestIndex];
                currentTest->ShowUI(showUI);
                currentScreen = currentTest;
            }
        }
    }
    else
    {
        if (showUI)
        {
            reportScreen->SetTestChain(testChain);
            currentScreen = reportScreen;
        }
        else
        {
            Logger::Info("Finish all tests.");
            Core::Instance()->Quit();
        }
    }
}