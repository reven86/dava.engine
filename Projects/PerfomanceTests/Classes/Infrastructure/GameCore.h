#ifndef __GAMECORE_H__
#define __GAMECORE_H__

#include "DAVAEngine.h"
#include "TeamCityTestsOutput.h"
#include "Platform/DeviceInfo.h"

#include "Infrastructure/Controller/TestFlowController.h"
#include "Infrastructure/Controller/TestChainFlowController.h"
#include "Infrastructure/Controller/SingleTestFlowController.h"
#include "Infrastructure/Settings/GraphicsDetect.h"

#include <fstream>

class GameCore : public ApplicationCore
{
public:
    GameCore();

    static GameCore* Instance()
    {
        return (GameCore*)DAVA::Core::GetApplicationCore();
    };

    void OnAppStarted() override;
    void OnAppFinished() override;

    void OnSuspend() override;
    void OnResume() override;
    
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    void OnBackground() override;
    void OnForeground() override;
    void OnDeviceLocked() override;
#endif

    void BeginFrame() override;
    void EndFrame() override;

private:
    void InitScreenController();
    void RegisterTests();
    void ReadSingleTestParams(BaseTest::TestParams& params);
    void LoadMaps(const String& testName, Vector<std::pair<String, String>>& maps);

    String GetDeviceName();

    Vector<BaseTest*> testChain;
    std::unique_ptr<TestFlowController> testFlowController;

    TeamcityTestsOutput teamCityOutput;
    BaseTest::TestParams defaultTestParams;
};



#endif // __GAMECORE_H__