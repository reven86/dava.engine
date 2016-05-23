#ifndef __DAVAENGINE_AUTOTESTING_SYSTEM_H__
#define __DAVAENGINE_AUTOTESTING_SYSTEM_H__

#include "DAVAConfig.h"

#ifdef __DAVAENGINE_AUTOTESTING__

#include "DAVAEngine.h"
#include "Base/Singleton.h"
#include "FileSystem/FileSystem.h"

#include "Autotesting/AutotestingSystemLua.h"

#include "Platform/DateTime.h"

namespace DAVA
{
class Image;
class AutotestingSystemLuaDelegate;
class AutotestingSystemLua;
class AutotestingSystem : public Singleton<AutotestingSystem>, public ScreenShotCallbackDelegate
{
public:
    AutotestingSystem();
    ~AutotestingSystem();

    void OnAppStarted();
    void OnAppFinished();

    void Update(float32 timeElapsed);
    void Draw();

    void OnInit();
    inline bool IsInit()
    {
        return isInit;
    };

    void InitLua(AutotestingSystemLuaDelegate* _delegate);

    void OnScreenShot(Image* image) override;

    void RunTests();

    // Parameters from DB
    void FetchParametersFromDB();
    void FetchParametersFromIdYaml();
    void SetUpConnectionToDB();
    RefPtr<KeyedArchive> GetIdYamlOptions();

    void InitializeDevice();

    // Test organization
    void OnTestStart(const String& testName);
    void OnStepStart(const String& stepName);
    void OnStepFinished();
    void OnTestStarted();
    void OnError(const String& errorMessage = "");
    void ForceQuit(const String& logMessage = "");
    void OnTestsFinished();

    // helpers
    void OnInput(const UIEvent& input);

    inline Vector2 GetMousePosition()
    {
        return mouseMove.point;
    };
    bool FindTouch(int32 id, UIEvent& touch);
    bool IsTouchDown(int32 id);

    const String& GetScreenShotName();
    void MakeScreenShot();
    bool GetIsScreenShotSaving() const;

    // DB Master-Helper relations

    String GetTestId()
    {
        return Format("Test%03d", testIndex);
    };
    String GetStepId()
    {
        return Format("Step%03d", stepIndex);
    };
    String GetLogId()
    {
        return Format("Message%03d", logIndex);
    };

    String GetCurrentTimeString();
    String GetCurrentTimeMsString();

    inline AutotestingSystemLua* GetLuaSystem()
    {
        return luaSystem;
    };

    static String ResolvePathToAutomation(const String& automationPath);

protected:
    void OnScreenShotInternal(Image* image);
    AutotestingSystemLua* luaSystem;
    //DB
    void ExitApp();

private:
    bool isScreenShotSaving = false;

public:
    uint64 startTimeMS;

    bool isInit;
    bool isRunning;
    bool needExitApp;
    float32 timeBeforeExit;

    String projectName;
    String groupName;
    String deviceName;
    String testsDate;
    String runId;
    int32 testIndex;
    int32 stepIndex;
    int32 logIndex;

    String testDescription;
    String testFileName;
    String testFilePath;

    String buildDate;
    String buildId;
    String branch;
    String framework;
    String branchRev;
    String frameworkRev;

    bool isDB;
    bool needClearGroupInDB;

    bool isMaster;
    int32 requestedHelpers;
    String masterId; // for communication
    String masterTask;
    int32 masterRunId;
    bool isRegistered;
    bool isWaiting;
    bool isInitMultiplayer;
    String multiplayerName;
    float32 waitTimeLeft;
    float32 waitCheckTimeLeft;

    Map<int32, UIEvent> touches;
    UIEvent mouseMove;

    String screenShotName;
};

inline bool AutotestingSystem::GetIsScreenShotSaving() const
{
    return isScreenShotSaving;
}
};

#endif //__DAVAENGINE_AUTOTESTING__

#endif //__DAVAENGINE_AUTOTESTING_SYSTEM_H__