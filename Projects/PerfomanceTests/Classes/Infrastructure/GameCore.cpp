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
    new GraphicsDetect();
    GraphicsDetect::Instance()->ReloadSettings();
    SoundSystem::Instance()->InitFromQualitySettings();

    defaultTestParams.startTime = 0;
    defaultTestParams.endTime = 120000;
    defaultTestParams.targetTime = 120000;
    
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
    GraphicsDetect::Instance()->Release();
    SafeRelease(testFlowController);
    

    for(BaseTest* test : testChain)
	{
		SafeRelease(test);
	}

    Logger::Instance()->RemoveCustomOutput(&teamCityOutput);
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
void GameCore::OnDeviceLocked()
{
}

void GameCore::OnBackground()
{
}

void GameCore::OnForeground()
{
    ApplicationCore::OnForeground();
}

#endif //#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)


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
    testChain.push_back(new AsiaPerformanceTest(defaultTestParams));
    testChain.push_back(new GlobalPerformanceTest(defaultTestParams));
}

String GameCore::GetDeviceName()
{
    String device = "device_";

#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)
    device += DeviceInfo::GetManufacturer() + DeviceInfo::GetModel();
#else
    device += UTF8Utils::EncodeToUTF8(DeviceInfo::GetName());
#endif

    std::replace(device.begin(), device.end(), ' ', '_');
    std::replace(device.begin(), device.end(), '.', '_');
    
    return device;
}

void GameCore::InitScreenController()
{
    Random::Instance()->Seed(0);

    Logger::Instance()->AddCustomOutput(&teamCityOutput);
    String deviceName = GetDeviceName();
    Logger::Info(deviceName.c_str());
    
    BaseTest::TestParams singleTestParams = defaultTestParams;
    String testForRun;

	bool chooserFound = CommandLineParser::Instance()->CommandIsFound("-chooser");
    bool testFound = CommandLineParser::Instance()->CommandIsFound("-test");
    bool universalTest = CommandLineParser::Instance()->CommandIsFound("-universal-test");
    bool withoutUIFound = CommandLineParser::Instance()->CommandIsFound("-without-ui");

    bool testTimeFound = CommandLineParser::Instance()->CommandIsFound("-test-time");
    bool startTimeFound = CommandLineParser::Instance()->CommandIsFound("-statistic-start-time");
    bool endTimeFound = CommandLineParser::Instance()->CommandIsFound("-statistic-end-time");
    
    bool testFramesFound = CommandLineParser::Instance()->CommandIsFound("-test-frames");
    bool frameDeltaFound = CommandLineParser::Instance()->CommandIsFound("-frame-delta");

    bool debugFrameFound = CommandLineParser::Instance()->CommandIsFound("-debug-frame");
    bool maxDeltaFound = CommandLineParser::Instance()->CommandIsFound("-max-delta");

    if (universalTest)
    {
        String universalTest = CommandLineParser::Instance()->GetCommandParamAdditional("-universal-test", 0);
        
        testChain.clear();
        
        YamlParser* parser = YamlParser::Create("~res:/maps.yaml");
        
        DVASSERT_MSG(parser, "can't open ~res:/maps.yaml");
        
        YamlNode* rootNode = parser->GetRootNode();
        if(rootNode)
        {
            int32 sz = rootNode->GetCount();
            
            for(int32 i = 0; i < sz; ++i)
            {
                testChain.push_back(new UniversalTest(rootNode->GetItemKeyName(i), defaultTestParams));
            }
        }
        
        SafeRelease(parser);
        
        if(!universalTest.empty())
        {
            testForRun = universalTest;
        }
    }
    
    if(testFound)
    {
        testForRun = CommandLineParser::Instance()->GetCommandParamAdditional("-test", 0);
    }
    
	if (chooserFound)
	{
        testFlowController = new SingleTestFlowController(!withoutUIFound);
	}
    else if (!testForRun.empty())
    {
        if (testTimeFound)
        {
            String testTimeParam = CommandLineParser::Instance()->GetCommandParamAdditional("-test-time", 0);
            singleTestParams.targetTime = std::atoi(testTimeParam.c_str());
            
            if(singleTestParams.targetTime < 0)
            {
                Logger::Error("Incorrect params. TargetTime < 0");
                Core::Instance()->Quit();
            }
        }
        
        if(startTimeFound)
        {
            if(!endTimeFound)
            {
                Logger::Error("Incorrect params. Set end time for range");
                Core::Instance()->Quit();
            }
            
            String startTime = CommandLineParser::Instance()->GetCommandParamAdditional("-statistic-start-time", 0);
            String endTime = CommandLineParser::Instance()->GetCommandParamAdditional("-statistic-end-time", 0);
            
            singleTestParams.startTime = std::atoi(startTime.c_str());
            singleTestParams.endTime = std::atoi(endTime.c_str());
            
            int32 timeRange = singleTestParams.endTime - singleTestParams.startTime;
            
            if(timeRange < 100 || singleTestParams.startTime < 0)
            {
                Logger::Error("Incorrect params. Too small time range");
                Core::Instance()->Quit();
            }
        }
        
        if (testFramesFound)
        {
            if(!frameDeltaFound)
            {
                Logger::Error("Incorrect params. Set debug frame number");
                Core::Instance()->Quit();
            }
            
            String testFramesParam = CommandLineParser::Instance()->GetCommandParamAdditional("-test-frames", 0);
            String frameDeltaParam = CommandLineParser::Instance()->GetCommandParamAdditional("-frame-delta", 0);

            singleTestParams.targetFramesCount = std::atoi(testFramesParam.c_str());
            singleTestParams.targetFrameDelta = std::atof(frameDeltaParam.c_str());
            
            if(singleTestParams.targetFrameDelta < 0.0f)
            {
                Logger::Error("Incorrect params. TargetFrameDelta < 0");
                Core::Instance()->Quit();
            }
            
            if(singleTestParams.targetFramesCount < 0)
            {
                Logger::Error("Incorrect params. TargetFramesCount < 0");
                Core::Instance()->Quit();
            }
        }
        
        if(frameDeltaFound)
        {
            String frameDeltaParam = CommandLineParser::Instance()->GetCommandParamAdditional("-frame-delta", 0);
            singleTestParams.targetFrameDelta = std::atof(frameDeltaParam.c_str());
            
            if(singleTestParams.targetFrameDelta < 0.0f)
            {
                Logger::Error("Incorrect params. TargetFrameDelta < 0");
                Core::Instance()->Quit();
            }
        }

        if (debugFrameFound)
        {
            String debugFrameParam = CommandLineParser::Instance()->GetCommandParamAdditional("-debug-frame", 0);
            singleTestParams.frameForDebug = std::atoi(debugFrameParam.c_str());
            
            if(singleTestParams.frameForDebug < 0)
            {
                Logger::Error("Incorrect params. DebugFrame < 0");
                Core::Instance()->Quit();
            }
        }
        
        if (maxDeltaFound)
        {
            String maxDeltaParam = CommandLineParser::Instance()->GetCommandParamAdditional("-max-delta", 0);
            singleTestParams.maxDelta = std::atof(maxDeltaParam.c_str());
            
            if(singleTestParams.maxDelta < 0.0f)
            {
                Logger::Error("Incorrect params. MaxDelta < 0");
                Core::Instance()->Quit();
            }
        }

        testFlowController = new SingleTestFlowController(testForRun, singleTestParams, !withoutUIFound);
        
        Logger::Instance()->Info(DAVA::Format("Test %s params ", testForRun.c_str()).c_str());
        Logger::Instance()->Info(DAVA::Format("Target time : %d", singleTestParams.targetTime).c_str());
        Logger::Instance()->Info(DAVA::Format("Statistic start time : %d", singleTestParams.startTime).c_str());
        Logger::Instance()->Info(DAVA::Format("Statistic end time : %d", singleTestParams.endTime).c_str());
        Logger::Instance()->Info(DAVA::Format("Target frames count : %d", singleTestParams.targetFramesCount).c_str());
        Logger::Instance()->Info(DAVA::Format("Target frame delta : %f", singleTestParams.targetFrameDelta).c_str());
        Logger::Instance()->Info(DAVA::Format("Frame for debug : %d", singleTestParams.frameForDebug).c_str());
        Logger::Instance()->Info(DAVA::Format("Max delta : %f", singleTestParams.maxDelta).c_str());
    }
	else if (withoutUIFound)
	{
        testFlowController = new TestChainFlowController(false);
	} 
	else
	{
		testFlowController = new TestChainFlowController(true);
	}

    testFlowController->Init(testChain);
}




