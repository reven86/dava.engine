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

GameCore::GameCore() :
currentScreen(nullptr),
screenIndex(0)

{
}

GameCore::~GameCore()
{
	for each (BaseScreen* screen in screenChain)
	{
		screen->Release();
	}

	for each (BaseTest* test in testChain)
	{
		test->Release();
	}
}

void GameCore::OnAppStarted()
{
	RegisterTests();

	if (testChain.size() > 0)
	{
		InitScreenChain();

		currentScreen = screenChain[screenIndex];
		currentScreen->OnStart(params);
	}
	else
	{
		Core::Instance()->Quit();
	}
}
 
void GameCore::OnAppFinished()
{
}

void GameCore::OnSuspend()
{
//    Logger::Debug("GameCore::OnSuspend");
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    ApplicationCore::OnSuspend();
#endif

}

void GameCore::OnResume()
{
    Logger::Debug("GameCore::OnResume");
    ApplicationCore::OnResume();
}


#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)
void GameCore::OnDeviceLocked()
{
//    Logger::Debug("GameCore::OnDeviceLocked");
    //Core::Instance()->Quit();
}

void GameCore::OnBackground()
{
    Logger::Debug("GameCore::OnBackground");
}

void GameCore::OnForeground()
{
    Logger::Debug("GameCore::OnForeground");
    ApplicationCore::OnForeground();
}

#endif //#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)


void GameCore::BeginFrame()
{
	currentScreen->BeginFrame();
}

void GameCore::EndFrame()
{
	currentScreen->EndFrame();

	if (currentScreen->IsFinished())
	{
		screenIndex++;
		currentScreen->OnFinish(params);

		if (screenIndex < screenChain.size())
		{
			currentScreen = screenChain[screenIndex];
			currentScreen->OnStart(params);
		}
		else
		{
			Core::Instance()->Quit();
		}
	}
}

void GameCore::Update(float32 timeElapsed)
{
	currentScreen->Update(timeElapsed);

	KeyboardDevice& keyb = DAVA::InputSystem::Instance()->GetKeyboard();

#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__)
	bool backPressed = keyb.IsKeyPressed(DAVA::DVKEY_ESCAPE);
	if (backPressed)
	{
		Core::Instance()->Quit();
	}

#endif

}

void GameCore::Draw()	
{
	currentScreen->Draw();
}

void GameCore::RegisterTests()
{
	testChain.push_back(new PerfomanceTest(300, 0.016f, 200));
	//testChain.push_back(new PerfomanceTest(300, 0.016f, 200));
	//testChain.push_back(new PerfomanceTest(300, 0.016f, 200));
}

void GameCore::InitScreenChain()
{
	bool allTests = CommandLineParser::Instance()->CommandIsFound("all");
	bool allWithUI = CommandLineParser::Instance()->CommandIsFound("all_ui");

	if (allTests)
	{
		screenChain.push_back(new TestChainScreen(testChain));
	}
	else if (allWithUI)
	{
		screenChain.push_back(new TestChainScreen(testChain));
		screenChain.push_back(new ReportScreen(testChain));
	} 
	else
	{
		screenChain.push_back(new TestChooserScreen(testChain));
		screenChain.push_back(new SingleTestScreen());
	}
}




