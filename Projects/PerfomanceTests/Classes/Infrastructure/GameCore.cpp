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

GameCore::GameCore() : reportScreen(nullptr), currentScreen(nullptr)
{
}

GameCore::~GameCore()
{
	for each (BaseTest* test in testsChain)
	{
		test->Release();
	}
}

void GameCore::OnAppStarted()
{
	RegisterTests();

	if (testsChain.size() > 0)
	{
		InitTestProcessor();
	}
	else
	{
		Core::Instance()->Quit();
	}
}

void GameCore::OnAppFinished()
{
    DAVA::Logger::Instance()->RemoveCustomOutput(&teamCityOutput);
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

	if (currentScreen->IsFinished() && reportScreen == nullptr)
	{
		reportScreen = new ReportScreen(testsChain);
		reportScreen->CreateReportScreen();

		currentScreen->Release();
		currentScreen = reportScreen;
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
	testsChain.push_back(new PerfomanceTest());
	testsChain.push_back(new PerfomanceTest());
	testsChain.push_back(new PerfomanceTest());
}

void GameCore::InitTestProcessor()
{
	bool fixedFrames = CommandLineParser::Instance()->CommandIsFound("-fixed_frames");
	bool fixedTime = CommandLineParser::Instance()->CommandIsFound("-fixed_time");

	bool debugTest = CommandLineParser::Instance()->CommandIsFound("-run_test_d");
	bool singleTestTime = CommandLineParser::Instance()->CommandIsFound("-run_test_t");
	bool singleTestFrame = CommandLineParser::Instance()->CommandIsFound("-run_test_f");

	if (fixedFrames || fixedTime)
	{
		uint32 fixedFramesCount = 0;
		float32 fixedDelta = 0.0f;
		uint32 fixedTime = 0;

		if (fixedFrames)
		{
			String frames = CommandLineParser::Instance()->GetCommandParamAdditional("-fixed_frames", 0);
			String delta = CommandLineParser::Instance()->GetCommandParamAdditional("-fixed_frames", 1);

			fixedFramesCount = atoi(frames.c_str());
			fixedDelta = strtof(delta.c_str(), nullptr);
		}
		else
		{
			String time = CommandLineParser::Instance()->GetCommandParamAdditional("-fixed_time", 0);
			fixedTime = atoi(time.c_str());
		}

		currentScreen = new TestChainScreen(testsChain, fixedTime, fixedFramesCount, fixedDelta);
	}

	if (singleTestTime || debugTest || singleTestFrame)
	{
		String testName;
		uint32 targetFrame = 0;

		uint32 fixedFramesCount = 0;
		float32 fixedDelta = 0.0f;
		uint32 fixedTime = 0;

		if (singleTestTime)
		{	
			testName = CommandLineParser::Instance()->GetCommandParamAdditional("-run_test_t", 0);
			String time = CommandLineParser::Instance()->GetCommandParamAdditional("-run_test_t", 1);
			fixedTime = atoi(time.c_str());
		}

		if (singleTestFrame)
		{
			testName = CommandLineParser::Instance()->GetCommandParamAdditional("-run_test_f", 0);
			String frames = CommandLineParser::Instance()->GetCommandParamAdditional("-run_test_f", 1);
			String delta = CommandLineParser::Instance()->GetCommandParamAdditional("-run_test_f", 2);

			fixedFramesCount = atoi(frames.c_str());
			fixedDelta = strtof(delta.c_str(), nullptr);
		}

		if (debugTest)
		{
			testName = CommandLineParser::Instance()->GetCommandParamAdditional("-run_test_d", 0);
			String target = CommandLineParser::Instance()->GetCommandParamAdditional("-run_test_d", 1);
			String delta = CommandLineParser::Instance()->GetCommandParamAdditional("-run_test_d", 2);

			targetFrame = atoi(target.c_str());
			fixedDelta = strtof(delta.c_str(), nullptr);
		}
	
		BaseTest* testForRun = nullptr;

		for each (BaseTest* test in testsChain)
		{
			if (test->GetName() == testName)
			{
				testForRun = test;
				break;
			}
		}

		currentScreen = new SingleTestScreen(testForRun, fixedTime, fixedFramesCount, fixedDelta, targetFrame);
	}

	Logger::Instance()->AddCustomOutput(&teamCityOutput);
}

void GameCore::LogMessage(const String &message)
{
    DAVA::Logger::Error(message.c_str());
}






