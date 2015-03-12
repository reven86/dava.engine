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


#ifndef __DAVAENGINE_AUTOTESTING_SYSTEM_H__
#define __DAVAENGINE_AUTOTESTING_SYSTEM_H__

#include "DAVAConfig.h"

#ifdef __DAVAENGINE_AUTOTESTING__

#include "DAVAEngine.h"
#include "Base/Singleton.h"
#include "FileSystem/FileSystem.h"

#include "Autotesting/AutotestingSystemLua.h"

#include "Render/RenderManager.h"
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
    inline bool IsInit() { return isInit; };

	void InitLua(AutotestingSystemLuaDelegate* _delegate);

	void OnScreenShot(Image *image) override;
    
    void RunTests();
    
	// Parameters from DB
	void FetchParametersFromDB();
	void FetchParametersFromIdTxt();
	void SetUpConnectionToDB();

	void InitializeDevice(const String & device);

	// Test organization
	void OnTestStart(const String &testName);
	void OnStepStart( const String & stepName );
	void OnStepFinished();
	void OnTestsSatrted();
    void OnError(const String & errorMessage = "");
	//void OnMessage(const String & logMessage = "");
	void ForceQuit(const String & logMessage = "");
    void OnTestsFinished();
    
    // helpers
    void OnInput(const UIEvent &input);
    
    inline Vector2 GetMousePosition() { return mouseMove.point; };
    bool FindTouch(int32 id, UIEvent &touch);
    bool IsTouchDown(int32 id);

	const String & GetScreenShotName();
	void MakeScreenShot(bool skipScreenshot);

    // DB Master-Helper relations
    //void InitMultiplayer(bool _isMaster);
    //void RegisterMasterInDB(int32 helpersCount);
    //void RegisterHelperInDB();

	String GetTestId() { return Format("Test%03d", testIndex); };
	String GetStepId() { return Format("Step%03d", stepIndex); };
	String GetLogId() { return  Format("Message%03d", logIndex); };
    
	String GetCurrentTimeString();
	String GetCurrentTimeMsString();

	inline AutotestingSystemLua* GetLuaSystem() { return luaSystem; };
protected:
	AutotestingSystemLua * luaSystem;
//DB
    /*void SetUpTestArchive();

	KeyedArchive *FindTestArchive(MongodbUpdateObject* dbUpdateObject, const String &testId);
    KeyedArchive *FindStepArchive(KeyedArchive *testArchive, const String &stepId);
    
    bool CheckSavedObjectInDB(MongodbUpdateObject *dbUpdateObject);
    bool CheckKeyedArchivesEqual(const String &name, KeyedArchive* firstKeyedArchive, KeyedArchive* secondKeyedArchive);

    void AddTestResult(const String &text, bool isPassed, const String & error = "");
    void SaveTestToDB();
    void SaveTestStepToDB(const String &stepDescription, bool isPassed, const String &error = "");
    void SaveTestStepLogEntryToDB(const String &type, const String &time, const String &message);*/
	void SaveScreenShotNameToDB();

    /*String ReadMasterIDFromDB(); //TODO: get first available master
    
    bool CheckMasterHelpersReadyDB();
        
    int32 GetIndexInFileList(FileList &fileList, int32 index);*/
    
    void ExitApp();
	
public:
	uint64 startTimeMS;

    bool isInit;
    bool isRunning;
    bool needExitApp;
    float32 timeBeforeExit;
    
    String projectName;
    String groupName;
	String deviceId;
	String deviceName;
    String testsDate;
	String runId;
    int32 testIndex;
    int32 stepIndex;
    int32 logIndex;

    String testName;
    String testFileName;
    String testFilePath;

	String buildDate;
	String buildId;
	String branch;
	String framework;
	String branchRev;
	String frameworkRev;

//    struct TestResult
//    {
//        TestResult(const String &_name, bool _isPassed, const String &_error) : name(_name), isPassed(_isPassed), error(_error) {}
//        
//        String name;
//        bool isPassed;
//        String error;
//    };
//    Vector< TestResult > testResults;

    bool isDB;
    bool needClearGroupInDB;
	bool skipScreenshot;
    
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

};

#endif //__DAVAENGINE_AUTOTESTING__

#endif //__DAVAENGINE_AUTOTESTING_SYSTEM_H__