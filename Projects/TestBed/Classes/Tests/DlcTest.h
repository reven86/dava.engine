#ifndef __TEST_SCREEN_H__
#define __TEST_SCREEN_H__

#include "DAVAEngine.h"
#include "DLC/DLC.h"
#include "Infrastructure/BaseScreen.h"

using namespace DAVA;

struct DLCCrashTest
{
    uint64 cancelTimeout;
    uint64 exitTimeout;
    uint32 retryCount;

    DAVA::FilePath testingFileFlag;
    DAVA::String dbObjectId;

    bool forceExit;
    bool inExitMode;

    Thread* exitThread;

    void Init(const DAVA::FilePath& workingDir, const DAVA::FilePath& destinationDir);
    void Update(float32 timeElapsed, DLC* dlc);

    void ExitThread(BaseObject* caller, void* callerData, void* userData);
};

class DlcTest : public BaseScreen
{
public:
    DlcTest();

protected:
    ~DlcTest()
    {
    }

public:
    void LoadResources() override;
    void UnloadResources() override;
    void OnActive() override;

    void Update(float32 timeElapsed) override;
    void Draw(const UIGeometricData& geometricData) override;

private:
    void UpdateInfoStr();
    void SetInternalDlServer(BaseObject* obj, void* data, void* callerData);
    void SetExternalDlServer(BaseObject* obj, void* data, void* callerData);
    void IncDlThreads(BaseObject* obj, void* data, void* callerData);
    void DecDlThreads(BaseObject* obj, void* data, void* callerData);
    void Start(BaseObject* obj, void* data, void* callerData);
    void Cancel(BaseObject* obj, void* data, void* callerData);
    void Restart(BaseObject* obj, void* data, void* callerData);

protected:
    String gameVersion = "dlcdevtest";
    String currentDownloadUrl;

    DAVA::FilePath workingDir;
    DAVA::FilePath sourceDir;
    DAVA::FilePath destinationDir;

    UITextField* gameVersionIn = nullptr;
    UIStaticText* infoText = nullptr;
    WideString infoStr;

    uint32 downloadTreadsCount = 4;

    UIStaticText* staticText = nullptr;
    UIControl* animControl = nullptr;
    UIControl* progressControl = nullptr;

    float32 angle = 0;
    float32 lastUpdateTime = 0.f;
    uint32 lastDLCState = 0;

    DLC* dlc = nullptr;
    DLCCrashTest crashTest;
};

#endif
