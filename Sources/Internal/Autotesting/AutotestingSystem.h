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
#include "Database/MongodbClient.h"

#include "Autotesting/MongodbUpdateObject.h"

//#define AUTOTESTING_DB_HOST    "10.128.19.33"
#define AUTOTESTING_DB_HOST    "by2-buildmachine.wargaming.net"
//#define AUTOTESTING_DB_HOST    "10.128.128.5"
//#define AUTOTESTING_DB_HOST    "192.168.1.2"
#define AUTOTESTING_DB_PORT  27017
#define AUTOTESTING_DB_NAME  "Autotesting"

#if defined (__DAVAENGINE_MACOS__)
#define AUTOTESTING_PLATFORM_NAME  "MacOS"
#elif defined (__DAVAENGINE_IPHONE__)
#define AUTOTESTING_PLATFORM_NAME  "iOS"
#elif defined (__DAVAENGINE_WIN32__)
#define AUTOTESTING_PLATFORM_NAME  "Windows"
#elif defined (__DAVAENGINE_ANDROID__)
#define AUTOTESTING_PLATFORM_NAME  "Android"
#else
#define AUTOTESTING_PLATFORM_NAME  "Unknown"
#endif //PLATFORMS    

#include "Render/RenderManager.h"

namespace DAVA
{

class Image;

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
    
    void SetProjectName(const String &_projectName);

	void OnScreenShot(Image *image);
    
    void RunTests();
    
	// multiplayer api
	void WriteState(const String & device, const String & state);
	void WriteCommand(const String & device, const String & state);

	String ReadState(const String & device);
	String ReadCommand(const String & device);

	void InitializeDevice(const String & device);

	// Test organization
	void OnTestStart(const String &testName);
	void OnStepStart( const String & stepName );
	void OnStepFinished();

	void Log(const String &level, const String &message);

    void OnTestsSatrted();
    void OnTestStep(const String & stepName, bool isPassed, const String & error = "");
    void OnError(const String & errorMessage = "");
	void OnMessage(const String & logMessage = "");
    void OnTestsFinished();
    
    // helpers
    void OnInput(const UIEvent &input);
    
    inline Vector2 GetMousePosition() { return mouseMove.point; };
    bool FindTouch(int32 id, UIEvent &touch);
    bool IsTouchDown(int32 id);

	// DB storing
	void WriteString(const String & name, const String & text);
	String ReadString(const String & name);

	String GetScreenShotName();
	void MakeScreenShot();

	bool SaveKeyedArchiveToDB(const String &archiveName, KeyedArchive *archive, const String &docName);

    // DB Master-Helper relations
    void InitMultiplayer(bool _isMaster);
    void RegisterMasterInDB(int32 helpersCount);
    void RegisterHelperInDB();
    
protected:
	String GetTestId(int32 index) { return Format("Test%03d", index); };
	String GetStepId(int32 index) { return Format("Step%03d", index); };
	String GetLogId(int32 index) { return  Format("Message%03d", index); };

	uint64 GetCurrentTimeMS();
	String GetCurrentTimeString();
	String GetCurrentTimeMsString();
//DB
	KeyedArchive *FindOrInsertRunArchive(MongodbUpdateObject* dbUpdateObject, const String &auxArg);
    void ClearTestInDB();
    
    KeyedArchive *FindOrInsertTestArchive(MongodbUpdateObject *dbUpdateObject, const String &testId);
    KeyedArchive *FindOrInsertTestStepArchive(KeyedArchive *testArchive, const String &stepId);
    KeyedArchive *FindOrInsertTestStepLogEntryArchive(KeyedArchive *testStepArchive, const String &logId);

	KeyedArchive *InsertTestArchive(MongodbUpdateObject* dbUpdateObject, const String &testId, const String &testName, bool needClearGroup);
	KeyedArchive *InsertStepArchive(KeyedArchive *testArchive, const String &stepId, const String &description);

	//KeyedArchive *FindTestArchive(MongodbUpdateObject* dbUpdateObject, const String &testId);
    KeyedArchive *FindStepArchive(KeyedArchive *testArchive, const String &stepId);
    
    bool SaveToDB(MongodbUpdateObject *dbUpdateObject);
    bool CheckSavedObjectInDB(MongodbUpdateObject *dbUpdateObject);
    bool CheckKeyedArchivesEqual(const String &name, KeyedArchive* firstKeyedArchive, KeyedArchive* secondKeyedArchive);

    bool ConnectToDB();
//    void AddTestResult(const String &text, bool isPassed, const String & error = "");
//    void SaveTestToDB();
    void SaveTestStepToDB(const String &stepDescription, bool isPassed, const String &error = "");
    void SaveTestStepLogEntryToDB(const String &type, const String &time, const String &message);
	void SaveScreenShotNameToDB();

    String ReadMasterIDFromDB(); //TODO: get first available master
    
    bool CheckMasterHelpersReadyDB();
    //
    
    int32 GetIndexInFileList(FileList &fileList, int32 index);
    
    void ExitApp();
	

	uint64 startTimeMS;

    bool isInit;
    bool isRunning;
    bool needExitApp;
    float32 timeBeforeExit;
    
    String projectName;
    String groupName;
	String device;
    uint32 testsId;
    uint32 testsDate;
    int32 testIndex;
    int32 stepIndex;
    int32 logIndex;

    String testName;
    String testFileName;
    String testFilePath;

	String deviceName;
//    struct TestResult
//    {
//        TestResult(const String &_name, bool _isPassed, const String &_error) : name(_name), isPassed(_isPassed), error(_error) {}
//        
//        String name;
//        bool isPassed;
//        String error;
//    };
//    Vector< TestResult > testResults;

    MongodbClient *dbClient;
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

    FilePath testReportsFolder;
    File* reportFile;

    Map<int32, UIEvent> touches;
    UIEvent mouseMove;

	String screenShotName;
};

};

#endif //__DAVAENGINE_AUTOTESTING__

#endif //__DAVAENGINE_AUTOTESTING_SYSTEM_H__