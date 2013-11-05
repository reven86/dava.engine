/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/
#include "Autotesting/AutotestingSystem.h"

#ifdef __DAVAENGINE_AUTOTESTING__

#include "Core/Core.h"
#include "Render/RenderHelper.h"
#include "FileSystem/FileList.h"
#include "Platform/DeviceInfo.h"
#include "FileSystem/KeyedArchive.h"

#include "AutotestingSystemLua.h"
#include "Autotesting/AutotestingDB.h"

namespace DAVA
{

AutotestingSystem::AutotestingSystem()
    : isInit(false)
    , isRunning(false)
    , needExitApp(false)
    , timeBeforeExit(0.0f)
    , testsDate(0)
    , testIndex(0)
    , isDB(false)
    , needClearGroupInDB(false)
    , groupName("default")
	, deviceId("not-initialized")
	, deviceName("not-initialized")
    , isMaster(true)
    , requestedHelpers(0)
    , isWaiting(false)
    , isRegistered(false)
    , waitTimeLeft(0.0f)
    , waitCheckTimeLeft(0.0f)
    , masterRunId(0)
    , isInitMultiplayer(false)
    , stepIndex(0)
    , logIndex(0)
    , startTimeMS(0)
{
    new AutotestingSystemLua();
	new AutotestingDB();
}

AutotestingSystem::~AutotestingSystem()
{
    AutotestingSystemLua::Instance()->Release();
	AutotestingDB::Instance()->Release();
}

// This method is called on application started and it handle autotest initialisation
void AutotestingSystem::OnAppStarted()
{
	Logger::Debug("AutotestingSystem::OnAppStarted");
	
	//isInit = true; //for debbug
	
	if(!isInit)
	{
		FetchParametersFromIdTxt();
		deviceName = GetDeviceName();
		//Logger::Debug("AutotestingSystem::OnAppStarted DeviceName=empty");
		
		SetUpConnectionToDB();

		FetchParametersFromDB();
		ClearTestInDB();
		
		String testFilePath = Format("~res:/Autotesting/Tests/%s/%s", groupName.c_str(), testFileName.c_str());
		AutotestingSystemLua::Instance()->InitFromFile(testFilePath);
		//ForceQuit("SUCCESS");
	}
	else
	{
		Logger::Error("AutotestingSystem::OnAppStarted App already initialized. Skip autotest initialization");
	}
}

void AutotestingSystem::OnAppFinished()
{
	AutotestingDB::Instance()->CloseConnection();
}
    
void AutotestingSystem::RunTests()
{
    if(!isInit) return;
    
    if(!isRunning)
    {
        isRunning = true;
        OnTestsSatrted();
    }
}
    
void AutotestingSystem::OnInit()
{
    if(!isInit)
    {
        isInit = true;
		AutotestingDB::Instance()->Log("DEBUG", "OnInit");
    }
}

String AutotestingSystem::GetDeviceName()
{ 
	/*
	if (AUTOTESTING_PLATFORM_NAME == "Windows")
	{
		return "pc";
	}
	else if (AUTOTESTING_PLATFORM_NAME == "MacOS")
	{
		return "macbook";
	}
	else
	{
		return Format("%s", DeviceInfo::GetName());
	}
	*/
	if (AUTOTESTING_PLATFORM_NAME == "Android")
	{
		return DeviceInfo::GetModel();
	}
	else
	{
		return WStringToString(DeviceInfo::GetName());
	}	
}
// Get test parameters from id.tx
void AutotestingSystem::FetchParametersFromIdTxt()
{
	Logger::Debug("AutotestingSystem::FetchParametersFromIdTxt");

	//TODO: read autotesting id from ~res:/Autotesting/autotesting.archive
	File* file = File::Create("~res:/Autotesting/id.txt", File::OPEN | File::READ);
	
	if(file)
	{
		if(!file->IsEof())
		{
			char tempBuf[1024];
			file->ReadLine(tempBuf, 1024);
			sscanf(tempBuf, "%d", &testsDate);
			
			if(!file->IsEof())
			{
				char projectCharName[1024];
				file->ReadLine(tempBuf, 1024);
				sscanf(tempBuf, "%s", &projectCharName);
				String pName = projectCharName;
				if(!pName.empty())
				{
					projectName = pName;
					isDB = true;
				}		

				Logger::Debug("AutotestingSystem::FetchParametersFromIdTxt date=%d, project=%s", testsDate, projectName.c_str());
			}
			else
			{
				SafeRelease(file);
				ForceQuit("Couldn't read test project from id.txt");
			}
		}
		else
		{
			SafeRelease(file);
			ForceQuit("Couldn't read test date from id.txt");
		}
	}
	else
	{
		ForceQuit("Couldn't open id.txt");
	}
}

// Get test parameters from autotesting db
void AutotestingSystem::FetchParametersFromDB()
{
	Logger::Debug("AutotestingSystem::FetchParametersFromDB");
	groupName = AutotestingDB::Instance()->GetStringTestParameter(deviceName, "Group");
	if (groupName == "not_found")
	{
		ForceQuit("Couldn't get Group parameter from DB.");
	}

	testFileName = AutotestingDB::Instance()->GetStringTestParameter(deviceName, "Filename");
	if (testFileName == "not_found")
	{
		ForceQuit("Couldn't get Filename parameter from DB.");
	}

	testIndex = AutotestingDB::Instance()->GetIntTestParameter(deviceName, "Number");
	if (testIndex == -9999)
	{
		ForceQuit("Couldn't get Number parameter from DB.");
	}

	Logger::Debug("AutotestingSystem::FetchParametersFromDB Group=%s Filename=%s TestIndex=%d", groupName.c_str(), testFileName.c_str(), testIndex);
}

// Read DB parameters from config file and set connection to it
void AutotestingSystem::SetUpConnectionToDB()
{
	KeyedArchive *option = new KeyedArchive();
	bool res = option->LoadFromYamlFile("~res:/Autotesting/dbConfig.yaml");
	if (!res)
	{
		DVASSERT(false);
	}

	String dbName = option->GetString("name");
	String dbAddress = option->GetString("hostname");
	int32 dbPort = option->GetInt32("port");
	Logger::Debug("AutotestingSystem::SetUpConnectionToDB %s[%s:%d]", dbName.c_str(), dbAddress.c_str(), dbPort);

	if(! AutotestingDB::Instance()->ConnectToDB(projectName, dbName, dbAddress, dbPort))
	{
		ForceQuit("Couldn't connect to Test DB");
	}
}

void AutotestingSystem::ClearTestInDB()
{
    MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
    
    // clear test
    String testId = GetTestId();
    KeyedArchive* currentTestArchive = AutotestingDB::Instance()->InsertTestArchive(dbUpdateObject, testId, testName, needClearGroupInDB);
    
    // Create document for step 0 -  'Precondition'
    String stepId = GetStepId();
    AutotestingDB::Instance()->InsertStepArchive(currentTestArchive, stepId, "Precondition");

    AutotestingDB::Instance()->SaveToDB(dbUpdateObject);
    SafeRelease(dbUpdateObject);
}
      
uint64 AutotestingSystem::GetCurrentTimeMS()
{
    uint64 timeAbsMs = SystemTimer::Instance()->FrameStampTimeMS();
    timeAbsMs -= startTimeMS;
    return timeAbsMs;
}

// Multiplayer API
void AutotestingSystem::InitializeDevice(const String & device)
{
	Logger::Debug("AutotestingSystem::InitializeDevice device=%s", device.c_str());
	deviceId = device.c_str();
}

String AutotestingSystem::GetCurrentTimeString()
{
    uint64 timeAbsMs = GetCurrentTimeMS();

    uint16 hours = (timeAbsMs/3600000)%12;
    uint16 minutes = (timeAbsMs/60000)%60;
    uint16 seconds = (timeAbsMs/1000)%60;
	//Logger::Debug("TIME: %02d:%02d:%02d", hours, minutes, seconds);
    return Format("%02d:%02d:%02d", hours, minutes, seconds);
}

String AutotestingSystem::GetCurrentTimeMsString()
{
	uint64 timeAbsMs = SystemTimer::Instance()->AbsoluteMS();
	uint16 hours = (timeAbsMs/3600000)%12;
	uint16 minutes = (timeAbsMs/60000)%60;
	uint16 seconds = (timeAbsMs/1000)%60;
	uint16 miliseconds = (timeAbsMs)%1000;
	return Format("%02d:%02d:%02d.%03d", hours, minutes, seconds, miliseconds);
}

void AutotestingSystem::OnTestStart(const String &_testName)
{
	Logger::Debug("AutotestingSystem::OnTestStart %s", _testName.c_str());
    testName = _testName;
    
    String testId = GetTestId();
    MongodbUpdateObject *dbUpdateObject = new MongodbUpdateObject();
    KeyedArchive *currentTestArchive = AutotestingDB::Instance()->FindOrInsertTestArchive(dbUpdateObject, testId);
    currentTestArchive->SetString("Name", testName);
    AutotestingDB::Instance()->SaveToDB(dbUpdateObject);
    SafeRelease(dbUpdateObject);
    
    AutotestingDB::Instance()->Log("DEBUG", Format("OnTestStart %s", testName.c_str()));
}

void AutotestingSystem::OnStepStart(const String &stepName)
{
	Logger::Debug("AutotestingSystem::OnStepStart %s", stepName.c_str());

	OnStepFinished();

	++stepIndex;
	String testId = GetTestId();
	String stepId = GetStepId();
	

	MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
	KeyedArchive* currentTestArchive = AutotestingDB::Instance()->FindOrInsertTestArchive(dbUpdateObject, testId);	

	AutotestingDB::Instance()->InsertStepArchive(currentTestArchive, stepId, stepName);

	AutotestingDB::Instance()->SaveToDB(dbUpdateObject);
	SafeRelease(dbUpdateObject);
}
    
void AutotestingSystem::OnStepFinished()
{
	Logger::Debug("AutotestingSystem::OnStepFinished");

	// Mark step as SUCCESS
	String testId = GetTestId();
	String stepId = GetStepId();
	logIndex = 0;

	MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
	KeyedArchive* currentTestArchive = AutotestingDB::Instance()->FindOrInsertTestArchive(dbUpdateObject, testId);
	if(currentTestArchive)
	{
		KeyedArchive* currentStepArchive = AutotestingDB::Instance()->FindOrInsertTestStepArchive(currentTestArchive, stepId);

		if (currentStepArchive)
		{
			currentStepArchive->SetBool("Success", true);
		}
	}
	else
	{
		OnError(Format("AutotestingSystem::OnStepFinished test %s not found", testId.c_str()));
	}

	AutotestingDB::Instance()->SaveToDB(dbUpdateObject);
	SafeRelease(dbUpdateObject);
}

void AutotestingSystem::SaveScreenShotNameToDB()
{
	Logger::Debug("AutotestingSystem::SaveScreenShotNameToDB %s", screenShotName.c_str());
	
	String testId = GetTestId();
	String stepId = GetStepId();

	MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
	KeyedArchive* currentTestArchive = AutotestingDB::Instance()->FindOrInsertTestArchive(dbUpdateObject, testId);
	KeyedArchive* currentStepArchive = AutotestingDB::Instance()->FindOrInsertTestStepArchive(currentTestArchive, stepId);

	currentStepArchive->SetString("screenshot", screenShotName);

	AutotestingDB::Instance()->SaveToDB(dbUpdateObject);
	SafeRelease(dbUpdateObject);
}
    
void AutotestingSystem::Update(float32 timeElapsed)
{
    if(!isInit) return;
     
    if(needExitApp)
    {
        timeBeforeExit -= timeElapsed;
        if(timeBeforeExit <= 0.0f)
        {
            needExitApp = false;
            Core::Instance()->Quit();
        }
        return;
    }

    if(isRunning)
    {
        AutotestingSystemLua::Instance()->Update(timeElapsed);
    }
}

void AutotestingSystem::Draw()
{
    if(!isInit) return;

    if(!touches.empty())
    {
        for(Map<int32, UIEvent>::iterator it = touches.begin(); it != touches.end(); ++it)
        {
            Vector2 point = it->second.point;
            RenderHelper::Instance()->DrawCircle(point, 25.0f);
        }
    }
    RenderHelper::Instance()->DrawCircle(GetMousePosition(), 15.0f);
}

void AutotestingSystem::OnTestsSatrted()
{
    Logger::Debug("AutotestingSystem::OnTestsStarted");
    
    startTimeMS = SystemTimer::Instance()->FrameStampTimeMS();
    
    AutotestingSystemLua::Instance()->StartTest();
}  
    
void AutotestingSystem::OnError(const String & errorMessage)
{
    Logger::Error("AutotestingSystem::OnError %s",errorMessage.c_str());
	
	if (isDB)
	{
		AutotestingDB::Instance()->Log("ERROR", errorMessage);
		//SaveTestStepLogEntryToDB("ERROR", GetCurrentTimeString(), errorMessage);
	
		MakeScreenShot();
		SaveScreenShotNameToDB();
    
		if (deviceId != "not-initialized")
		{
			AutotestingDB::Instance()->WriteState(deviceId, "error");
		}
	}

    ExitApp();
}

void AutotestingSystem::ForceQuit(const String & errorMessage)
{
	Logger::Error("AutotestingSystem::ForceQuit %s",errorMessage.c_str());
	
	AutotestingDB::Instance()->CloseConnection();
	
	ExitApp();
	
	exit(0);
}

void AutotestingSystem::MakeScreenShot()
{
	Logger::Debug("AutotestingSystem::MakeScreenShot");
	uint64 timeAbsMs = GetCurrentTimeMS();
    uint16 hours = (timeAbsMs/3600000)%24;
    uint16 minutes = (timeAbsMs/60000)%60;
    uint16 seconds = (timeAbsMs/1000)%60;
	screenShotName = Format("%s_%s_%s_%02d_%02d_%02d", AUTOTESTING_PLATFORM_NAME, deviceName.c_str(), groupName.c_str(), hours, minutes, seconds);

	RenderManager::Instance()->RequestGLScreenShot(this);
}

String AutotestingSystem::GetScreenShotName()
{
	Logger::Debug("AutotestingSystem::GetScreenShotName %s", screenShotName.c_str());
	return screenShotName.c_str();
}

void AutotestingSystem::OnScreenShot(Image *image)
{
	Logger::Debug("AutotestingSystem::OnScreenShot %s", screenShotName.c_str());
	uint64 startTime = SystemTimer::Instance()->AbsoluteMS();

	AutotestingDB::Instance()->UploadScreenshot(screenShotName, image);
	uint64 finishTime = SystemTimer::Instance()->AbsoluteMS();
	Logger::Debug("AutotestingSystem::OnScreenShot Upload: %d", finishTime - startTime);
}
    
void AutotestingSystem::OnTestsFinished()
{
    Logger::Debug("AutotestingSystem::OnTestsFinished");
    
	// Mark last step as SUCCESS
	OnStepFinished();

	if (deviceId != "not-initialized")
	{
		AutotestingDB::Instance()->WriteState(deviceId, "finished");
	}

	// Mark test as SUCCESS
	String testId = GetTestId();
	MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
	KeyedArchive* currentTestArchive = AutotestingDB::Instance()->FindOrInsertTestArchive(dbUpdateObject, testId);

	if (currentTestArchive)
	{
		currentTestArchive->SetBool("Success", true);
	}

	AutotestingDB::Instance()->SaveToDB(dbUpdateObject);
	SafeRelease(dbUpdateObject);

    ExitApp();
}

void AutotestingSystem::OnInput(const UIEvent &input)
{
    Logger::Debug("AutotestingSystem::OnInput %d phase=%d count=%d point=(%f, %f) physPoint=(%f,%f) key=%c",input.tid, input.phase, input.tapCount, input.point.x, input.point.y, input.physPoint.x, input.physPoint.y, input.keyChar);
    
    int32 id = input.tid;
    switch(input.phase)
    {
        case UIEvent::PHASE_BEGAN:
        {
            mouseMove = input;
            if(!IsTouchDown(id))
            {
                touches[id] = input;
            }
            else
            {
                Logger::Error("AutotestingSystemYaml::OnInput PHASE_BEGAN duplicate touch id=%d",id);
            }
        }
        break;
#if !defined(__DAVAENGINE_IPHONE__) && !defined(__DAVAENGINE_ANDROID__)
    case UIEvent::PHASE_MOVE:
        {
            mouseMove = input;
            if(IsTouchDown(id))
            {
                Logger::Error("AutotestingSystemYaml::OnInput PHASE_MOVE id=%d must be PHASE_DRAG",id);
            }
        }
            break;
#endif
        case UIEvent::PHASE_DRAG:
        {
            mouseMove = input;
            Map<int32, UIEvent>::iterator findIt = touches.find(id);
            if(findIt != touches.end())
            {
                findIt->second = input;
            }
            else
            {
                Logger::Error("AutotestingSystemYaml::OnInput PHASE_DRAG id=%d must be PHASE_MOVE",id);
            }
        }
            break;
        case UIEvent::PHASE_ENDED:
        {
            mouseMove = input;
            Map<int32, UIEvent>::iterator findIt = touches.find(id);
            if(findIt != touches.end())
            {
                touches.erase(findIt);
            }
            else
            {
                Logger::Error("AutotestingSystemYaml::OnInput PHASE_ENDED id=%d not found",id);
            }
        }
            break;
        default:
            //TODO: keyboard input
            break;
    }
}

bool AutotestingSystem::FindTouch(int32 id, UIEvent &touch)
{
    bool isFound = false;
    Map<int32, UIEvent>::iterator findIt = touches.find(id);
    if(findIt != touches.end())
    {
        isFound = true;
        touch = findIt->second;
    }
    return isFound;
}

bool AutotestingSystem::IsTouchDown(int32 id)
{
    return (touches.find(id) != touches.end());
}

void AutotestingSystem::ExitApp()
{
    if(!needExitApp)
    {
		isRunning = false;
		isWaiting = false;
        needExitApp = true;
        timeBeforeExit = 1.0f;
    }
}

// Multiplayer API


// Working with DB api

};

#endif //__DAVAENGINE_AUTOTESTING__