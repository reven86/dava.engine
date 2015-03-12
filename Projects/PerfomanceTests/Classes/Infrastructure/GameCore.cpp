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


#include "GameCore.h"

#include "Platform/DateTime.h"
#include "TexturePacker/CommandLineParser.h"
#include "Utils/Utils.h"

//$PERFOMANCETEST_INCLUDE

#include <fstream>
#include <algorithm>

using namespace DAVA;

GameCore::GameCore()
    :   testFlowController(nullptr)
    
{
}

void GameCore::OnAppStarted()
{
	RegisterTests();
	InitScreenController();

	if (testChain.empty())
	{
		Core::Instance()->Quit();
	}
}
 
void GameCore::OnAppFinished()
{
	testFlowController->Finish();
    SafeRelease(testFlowController);

	for each (BaseTest* test in testChain)
	{
		SafeRelease(test);
	}
}

void GameCore::BeginFrame()
{
	ApplicationCore::BeginFrame();
	testFlowController->BeginFrame();
}

void GameCore::EndFrame()
{
	ApplicationCore::EndFrame();
	testFlowController->EndFrame();
}

void GameCore::RegisterTests()
{
	//testChain.push_back(new PerfomanceTest(300, 0.016f, 200));
    //testChain.push_back(new GlobalPerformanceTest(1000, 0.016f, 200));
    testChain.push_back(new GlobalPerformanceTest(90000));
}

void GameCore::InitScreenController()
{
    Random::Instance()->Seed(0);

	bool allTests = CommandLineParser::Instance()->CommandIsFound("all");
	bool allWithUI = CommandLineParser::Instance()->CommandIsFound("all_ui");

	if (allTests)
	{
		testFlowController = new TestChainFlowController(false);
	}
	else if (allWithUI)
	{
		testFlowController = new TestChainFlowController(true);
	} 
	else
	{
		testFlowController = new SingleTestFlowController();
	}

    testFlowController->Init(testChain);
}




