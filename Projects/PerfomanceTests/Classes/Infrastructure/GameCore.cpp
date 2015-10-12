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
#include "CommandLine/CommandLineParser.h"
#include "Utils/Utils.h"

#include <fstream>
#include <algorithm>

#include "Tests/UniversalTest.h"
#include "Tests/MaterialsTest.h"

using namespace DAVA;

GameCore::GameCore()
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

    for(auto *test : testChain)
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
    // load material test
    Vector<std::pair<String, String> > scenes;
    LoadMaps(MaterialsTest::TEST_NAME, scenes);
    
    for(const auto& scene : scenes)
    {
        BaseTest::TestParams params = defaultTestParams;
        params.sceneName = scene.first;
        params.scenePath = scene.second;
        
        testChain.push_back(new MaterialsTest(params));
    }
    
    scenes.clear();
    
    // load universal test
    LoadMaps(UniversalTest::TEST_NAME, scenes);
    
    for(const auto& scene : scenes)
    {
        BaseTest::TestParams params = defaultTestParams;
        params.sceneName = scene.first;
        params.scenePath = scene.second;
        
        testChain.push_back(new UniversalTest(params));
    }
}

void GameCore::LoadMaps(const String& testName, Vector<std::pair<String, String>>& mapsVector)
{
    YamlParser* testsParser = YamlParser::Create("~res:/tests.yaml");
    DVASSERT_MSG(testsParser, "can't open ~res:/tests.yaml");
    
    YamlParser* mapsParser = YamlParser::Create("~res:/maps.yaml");
    DVASSERT_MSG(mapsParser, "can't open ~res:/maps.yaml");
    
    YamlNode* testsRootNode = testsParser->GetRootNode();
    YamlNode* mapsRootNode = mapsParser->GetRootNode();

    const auto& maps = testsRootNode->Get(testName)->AsVector();
    
    for(auto mapNameNode: maps)
    {
        const String& mapName = mapNameNode->AsString();
        const String& mapPath = mapsRootNode->Get(mapName)->AsString();
        
        mapsVector.push_back(std::pair<String, String>(mapName, mapPath));
    }
    
    SafeRelease(mapsParser);
    SafeRelease(testsParser);
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
    Logger::Info(GetDeviceName().c_str());

	bool chooserFound = CommandLineParser::Instance()->CommandIsFound("-chooser");
    bool testFound = CommandLineParser::Instance()->CommandIsFound("-test");
    bool withoutUIFound = CommandLineParser::Instance()->CommandIsFound("-without-ui");

    String testForRun;

    if (testFound)
    {
        testForRun = CommandLineParser::Instance()->GetCommandParamAdditional("-test", 0);
    }

    if (chooserFound)
    {
        testFlowController = std::unique_ptr<SingleTestFlowController>(new SingleTestFlowController("", defaultTestParams, !withoutUIFound));
    }
    else if (!testForRun.empty())
	{
        Logger::Instance()->Info(DAVA::Format("Test %s", testForRun.c_str()).c_str());

        BaseTest::TestParams singleTestParams = defaultTestParams;
        ReadSingleTestParams(singleTestParams);

        testFlowController = std::unique_ptr<SingleTestFlowController>(new SingleTestFlowController(testForRun, singleTestParams, !withoutUIFound));
    }
	else
	{
        testFlowController = std::unique_ptr<TestChainFlowController>(new TestChainFlowController(!withoutUIFound));
	} 

    testFlowController->Init(testChain);
}

void GameCore::ReadSingleTestParams(BaseTest::TestParams& params)
{
    bool testTimeFound = CommandLineParser::Instance()->CommandIsFound("-test-time");
    bool startTimeFound = CommandLineParser::Instance()->CommandIsFound("-statistic-start-time");
    bool endTimeFound = CommandLineParser::Instance()->CommandIsFound("-statistic-end-time");

    bool testFramesFound = CommandLineParser::Instance()->CommandIsFound("-test-frames");
    bool frameDeltaFound = CommandLineParser::Instance()->CommandIsFound("-frame-delta");

    bool debugFrameFound = CommandLineParser::Instance()->CommandIsFound("-debug-frame");
    bool maxDeltaFound = CommandLineParser::Instance()->CommandIsFound("-max-delta");

    if (testTimeFound)
    {
        String testTimeParam = CommandLineParser::Instance()->GetCommandParamAdditional("-test-time", 0);
        params.targetTime = std::atoi(testTimeParam.c_str());

        if (params.targetTime < 0)
        {
            Logger::Error("Incorrect params. TargetTime < 0");
            Core::Instance()->Quit();
        }
    }

    if (startTimeFound)
    {
        if (!endTimeFound)
        {
            Logger::Error("Incorrect params. Set end time for range");
            Core::Instance()->Quit();
        }

        String startTime = CommandLineParser::Instance()->GetCommandParamAdditional("-statistic-start-time", 0);
        String endTime = CommandLineParser::Instance()->GetCommandParamAdditional("-statistic-end-time", 0);

        params.startTime = std::atoi(startTime.c_str());
        params.endTime = std::atoi(endTime.c_str());

        int32 timeRange = params.endTime - params.startTime;

        if (timeRange < 100 || params.startTime < 0)
        {
            Logger::Error("Incorrect params. Too small time range");
            Core::Instance()->Quit();
        }
    }

    if (testFramesFound)
    {
        String testFramesParam = CommandLineParser::Instance()->GetCommandParamAdditional("-test-frames", 0);
        params.targetFramesCount = std::atoi(testFramesParam.c_str());

        if (params.targetFramesCount < 0)
        {
            Logger::Error("Incorrect params. TargetFramesCount < 0");
            Core::Instance()->Quit();
        }
    }

    if (frameDeltaFound)
    {
        String frameDeltaParam = CommandLineParser::Instance()->GetCommandParamAdditional("-frame-delta", 0);
        params.targetFrameDelta = std::atof(frameDeltaParam.c_str());

        if (params.targetFrameDelta < 0.0f)
        {
            Logger::Error("Incorrect params. TargetFrameDelta < 0");
            Core::Instance()->Quit();
        }
    }

    if (debugFrameFound)
    {
        String debugFrameParam = CommandLineParser::Instance()->GetCommandParamAdditional("-debug-frame", 0);
        params.frameForDebug = std::atoi(debugFrameParam.c_str());

        if (params.frameForDebug < 0)
        {
            Logger::Error("Incorrect params. DebugFrame < 0");
            Core::Instance()->Quit();
        }
    }

    if (maxDeltaFound)
    {
        String maxDeltaParam = CommandLineParser::Instance()->GetCommandParamAdditional("-max-delta", 0);
        params.maxDelta = std::atof(maxDeltaParam.c_str());

        if (params.maxDelta < 0.0f)
        {
            Logger::Error("Incorrect params. MaxDelta < 0");
            Core::Instance()->Quit();
        }
    }

    Logger::Instance()->Info(DAVA::Format("Target time : %d", params.targetTime).c_str());
    Logger::Instance()->Info(DAVA::Format("Statistic start time : %d", params.startTime).c_str());
    Logger::Instance()->Info(DAVA::Format("Statistic end time : %d", params.endTime).c_str());
    Logger::Instance()->Info(DAVA::Format("Target frames count : %d", params.targetFramesCount).c_str());
    Logger::Instance()->Info(DAVA::Format("Target frame delta : %f", params.targetFrameDelta).c_str());
    Logger::Instance()->Info(DAVA::Format("Frame for debug : %d", params.frameForDebug).c_str());
    Logger::Instance()->Info(DAVA::Format("Max delta : %f", params.maxDelta).c_str());
}


