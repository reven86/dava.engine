#pragma once

#include "DAVAEngine.h"
#include "TeamCityTestsOutput.h"
#include "Platform/DeviceInfo.h"

#include "Infrastructure/Controller/TestFlowController.h"
#include "Infrastructure/Controller/TestChainFlowController.h"
#include "Infrastructure/Controller/SingleTestFlowController.h"
#include "Infrastructure/Settings/GraphicsDetect.h"

namespace DAVA
{
class Engine;
}

class GameCore
{
public:
    GameCore(DAVA::Engine& e);

    static GameCore* Instance()
    {
        return instance;
    };

    void OnAppStarted();
    void OnAppFinished();
    void OnWindowCreated(DAVA::Window* w);
    void OnWindowResized(DAVA::Window* window, DAVA::float32 w, DAVA::float32 h,
                         DAVA::float32 scaleX, DAVA::float32 scaleY);

    void OnSuspend();
    void OnResume();

    void BeginFrame();
    void EndFrame();

    void Quit();

private:
    void InitScreenController();
    void RegisterTests();
    void ReadSingleTestParams(BaseTest::TestParams& params);
    void LoadMaps(const String& testName, Vector<std::pair<String, String>>& maps);
    void Cleanup();

    String GetDeviceName();

    Vector<BaseTest*> testChain;
    std::unique_ptr<TestFlowController> testFlowController;

    TeamcityPerformanceTestsOutput teamCityOutput;
    BaseTest::TestParams defaultTestParams;

    DAVA::Engine& engine;
    static GameCore* instance;
};