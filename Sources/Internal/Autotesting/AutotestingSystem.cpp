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


#include "Autotesting/AutotestingSystem.h"

#ifdef __DAVAENGINE_AUTOTESTING__

#include "Core/Core.h"
#include "Render/RenderHelper.h"
#include "FileSystem/FileList.h"
#include "Platform/DeviceInfo.h"
#include "Platform/DateTime.h"
#include "FileSystem/KeyedArchive.h"

#include "Autotesting/AutotestingSystemLua.h"
#include "Autotesting/AutotestingDB.h"

#include "Job/JobManager.h"

namespace DAVA
{

	AutotestingSystem::AutotestingSystem()
        : startTimeMS(0)
        , isInit(false)
        , isRunning(false)
        , needExitApp(false)
        , timeBeforeExit(0.0f)
        , projectName("")
        , groupName("default")
        , deviceId("not-initialized")
        , deviceName("not-initialized")
		, testsDate("not_found")
		, runId("not_found")
        , testIndex(0)
        , stepIndex(0)
        , logIndex(0)
        , testDescription("")
        , testFileName("")
        , testFilePath("")
		, buildDate("not_found")
        , buildId("zero-build")
        , branch("branch")
        , framework("framework")
        , branchRev("0")
        , frameworkRev("0")
        , isDB(true)
        , needClearGroupInDB(false)
        , isMaster(true)
        , requestedHelpers(0)
        , masterId("")
        , masterTask("")
        , masterRunId(0)
        , isRegistered(false)
        , isWaiting(false)
        , isInitMultiplayer(false)
        , multiplayerName("")
        , waitTimeLeft(0.0f)
        , waitCheckTimeLeft(0.0f)
        , luaSystem(nullptr)
	{
		new AutotestingDB();
	}

	AutotestingSystem::~AutotestingSystem()
	{
		SafeRelease(luaSystem);
		if (AutotestingDB::Instance())
			AutotestingDB::Instance()->Release();
	}

	void AutotestingSystem::InitLua(AutotestingSystemLuaDelegate* _delegate)
	{
		Logger::Info("AutotestingSystem::InitLua");
		DVASSERT(nullptr == luaSystem);
		luaSystem = new AutotestingSystemLua();
		luaSystem->SetDelegate(_delegate);
	}

    String AutotestingSystem::ResolvePathToAutomation(const String &automationPath)
    {
        String automationResolvedStrPath = "~res:" + automationPath;
        if (FileSystem::Instance()->Exists(FilePath(automationResolvedStrPath)))
        {
            return automationResolvedStrPath;
        }
#if defined(__DAVAENGINE_ANDROID__)
        FilePath automationResolvedPath = FileSystem::Instance()->GetPublicDocumentsPath() + automationPath;
#else
        FilePath automationResolvedPath = "~doc:" + automationPath;
#endif //#if defined(__DAVAENGINE_ANDROID__)
        if (FileSystem::Instance()->Exists(automationResolvedPath))
        {
            return automationResolvedPath.GetStringValue();
        }
        return "";
    }
    
	// This method is called on application started and it handle autotest initialisation
	void AutotestingSystem::OnAppStarted()
	{
		Logger::Info("AutotestingSystem::OnAppStarted");

		if (isInit)
		{
			Logger::Error("AutotestingSystem::OnAppStarted App already initialized.");
			return;
		}
		deviceName = AutotestingSystemLua::Instance()->GetDeviceName();
		FetchParametersFromIdYaml();

		if (isDB)
		{
			SetUpConnectionToDB();
			FetchParametersFromDB();
		}

		const String testFileLocation = Format("/Autotesting/Tests/%s/%s.lua", groupName.c_str(), testFileName.c_str());
        String testFileStrPath = ResolvePathToAutomation(testFileLocation);
		if (testFileStrPath.empty())
		{
			Logger::Error("AutotestingSystemLua::OnAppStarted: couldn't open %s", testFileLocation.c_str());
			return;
		}

		AutotestingDB::Instance()->WriteLogHeader();

		AutotestingSystemLua::Instance()->InitFromFile(testFileStrPath);
	}

	void AutotestingSystem::OnAppFinished()
	{
		Logger::Info("AutotestingSystem::OnAppFinished ");
		ExitApp();
	}

	void AutotestingSystem::RunTests()
	{
		if (!isInit || isRunning)
		{
			return;
		}
		isRunning = true;
		OnTestStarted();
	}

	void AutotestingSystem::OnInit()
	{
		DVASSERT(!isInit);
		isInit = true;
	}

	// Get test parameters from id.yaml
	void AutotestingSystem::FetchParametersFromIdYaml()
	{
		Logger::Info("AutotestingSystem::FetchParametersFromIdYaml");
		RefPtr<KeyedArchive> option = GetIdYamlOptions();

		buildId = option->GetString("BuildId");
		buildDate = option->GetString("Date");
		branch = option->GetString("Branch");
		framework = option->GetString("Framework");
		branchRev = option->GetString("BranchRev");
		frameworkRev = option->GetString("FrameworkRev");

		// Check is build fol local debugging.  By default: use DB.
		bool isLocalBuild = option->GetBool("LocalBuild", false);
		if (isLocalBuild)
		{
			groupName = option->GetString("Group", AutotestingDB::DB_ERROR_STR_VALUE);
			testFileName = option->GetString("Filename", AutotestingDB::DB_ERROR_STR_VALUE);
			isDB = false;
		}
	}

	RefPtr<KeyedArchive> AutotestingSystem::GetIdYamlOptions()
	{
		const String idYamlStrLocation = "/Autotesting/id.yaml";
        String idYamlStrPath = ResolvePathToAutomation(idYamlStrLocation);
		RefPtr<KeyedArchive> option(new KeyedArchive());
        if (idYamlStrPath.empty() || !option->LoadFromYamlFile(idYamlStrPath))
		{
			ForceQuit("Couldn't open file " + idYamlStrLocation);
		}

		return option;
	}

	// Get test parameters from autotesting db
	void AutotestingSystem::FetchParametersFromDB()
	{
		Logger::Info("AutotestingSystem::FetchParametersFromDB");
		AutotestingDB *db = AutotestingDB::Instance();
		groupName = db->GetStringTestParameter(deviceName, "Group");
		if (groupName == AutotestingDB::DB_ERROR_STR_VALUE)
		{
			ForceQuit("Couldn't get 'Group' parameter from DB.");
		}
		testFileName = db->GetStringTestParameter(deviceName, "Filename");
		if (groupName == AutotestingDB::DB_ERROR_STR_VALUE)
		{
			ForceQuit("Couldn't get 'Filename' parameter from DB.");
		}
		runId = db->GetStringTestParameter(deviceName, "RunId");
		if (runId == AutotestingDB::DB_ERROR_STR_VALUE)
		{
			ForceQuit("Couldn't get 'RunId' parameter from DB.");
		}
		testIndex = db->GetIntTestParameter(deviceName, "TestIndex");
		if (testIndex == AutotestingDB::DB_ERROR_INT_VALUE)
		{
			ForceQuit("Couldn't get TestIndex parameter from DB.");
		}
	}

	// Read DB parameters from config file and set connection to it
	void AutotestingSystem::SetUpConnectionToDB()
	{
        const String dbConfigLocation = "/Autotesting/dbConfig.yaml";
        String dbConfigStrPath = ResolvePathToAutomation(dbConfigLocation);
		KeyedArchive *option = new KeyedArchive();
		if (dbConfigStrPath.empty() || !option->LoadFromYamlFile(dbConfigStrPath))
		{
			ForceQuit("Couldn't open file " + dbConfigLocation);
		}

		String dbName = option->GetString("name");
		String dbAddress = option->GetString("hostname");
		String collection = option->GetString("collection");
		int32 dbPort = option->GetInt32("port");
		Logger::Info("AutotestingSystem::SetUpConnectionToDB %s -> %s[%s:%d]", collection.c_str(), dbName.c_str(), dbAddress.c_str(), dbPort);

		if (!AutotestingDB::Instance()->ConnectToDB(collection, dbName, dbAddress, dbPort))
		{
			ForceQuit("Couldn't connect to Test DB");
		}

		SafeRelease(option);
	}

	// Multiplayer API
	void AutotestingSystem::InitializeDevice(const String &device)
	{
		Logger::Info("AutotestingSystem::InitializeDevice device=%s", device.c_str());
		deviceId = device;
	}

	String AutotestingSystem::GetCurrentTimeString()
	{
		DateTime time = DateTime::Now();
		return Format("%02d-%02d-%02d", time.GetHour(), time.GetMinute(), time.GetSecond());
	}

	void AutotestingSystem::OnTestStart(const String &testDescription)
	{
		Logger::Info("AutotestingSystem::OnTestStart %s", testDescription.c_str());
		AutotestingDB::Instance()->Log("DEBUG", Format("OnTestStart %s", testDescription.c_str()));
		if (isDB)
			AutotestingDB::Instance()->SetTestStarted();
	}

	void AutotestingSystem::OnStepStart(const String &stepName)
	{
		Logger::Info("AutotestingSystem::OnStepStart %s", stepName.c_str());

		OnStepFinished();

		AutotestingDB::Instance()->Log("INFO", stepName);
	}

	void AutotestingSystem::OnStepFinished()
	{
		Logger::Info("AutotestingSystem::OnStepFinished");
		AutotestingDB::Instance()->Log("INFO", "Success");
	}


	void AutotestingSystem::Update(float32 timeElapsed)
	{
		if (!isInit)
		{
			return;
		}
		if (needExitApp)
		{
			timeBeforeExit -= timeElapsed;
			if (timeBeforeExit <= 0.0f)
			{
				needExitApp = false;
                String server = AutotestingDB::Instance()->GetStringTestParameter(deviceName, "Server");
                if (server != AutotestingDB::DB_ERROR_STR_VALUE)
                {
                    AutotestingSystemLua::Instance()->SetServerQueueState(server, 0);
                }
                JobManager::Instance()->WaitWorkerJobs();
				Core::Instance()->Quit();
			}
			return;
		}

		if (isRunning)
		{
			luaSystem->Update(timeElapsed);
		}
	}

	void AutotestingSystem::Draw()
	{
		if (!isInit)
		{
			return;
		}
		if (!touches.empty())
		{
			for (Map<int32, UIEvent>::iterator it = touches.begin(); it != touches.end(); ++it)
			{
				Vector2 point = it->second.point;
                RenderSystem2D::Instance()->DrawCircle(point, 25.0f, Color::White);
            }
        }
        RenderSystem2D::Instance()->DrawCircle(GetMousePosition(), 15.0f, Color::White);
    }

    void AutotestingSystem::OnTestStarted()
    {
        Logger::Info("AutotestingSystem::OnTestsStarted");
        startTimeMS = SystemTimer::Instance()->FrameStampTimeMS();
        luaSystem->StartTest();
    }

    void AutotestingSystem::OnError(const String &errorMessage)
	{
		Logger::Error("AutotestingSystem::OnError %s", errorMessage.c_str());

		AutotestingDB::Instance()->Log("ERROR", errorMessage);

		MakeScreenShot();
        
        AutotestingDB::Instance()->Log("ERROR", screenShotName);

		if (isDB && deviceId != "not-initialized")
		{
            AutotestingDB::Instance()->WriteState(deviceId, "error");
		}

		ExitApp();
	}

	void AutotestingSystem::ForceQuit(const String &errorMessage)
	{
		DVASSERT_MSG(false, errorMessage.c_str())
		Core::Instance()->Quit();
	}

	void AutotestingSystem::MakeScreenShot()
	{
		Logger::Info("AutotestingSystem::MakeScreenShot");
		String currentDateTime = GetCurrentTimeString();
		screenShotName = Format("%s_%s_%s_%d_%s", groupName.c_str(), testFileName.c_str(), runId.c_str(), testIndex, currentDateTime.c_str());
		Logger::Debug("AutotestingSystem::ScreenShotName %s", screenShotName.c_str());
        Renderer::RequestGLScreenShot(this);
    }

    const String& AutotestingSystem::GetScreenShotName()
    {
        Logger::Info("AutotestingSystem::GetScreenShotName %s", screenShotName.c_str());
        return screenShotName;
    }

    void AutotestingSystem::OnScreenShot(Image* image)
    {
        Function<void()> fn = Bind(&AutotestingSystem::OnScreenShotInternal, this, SafeRetain(image));
		JobManager::Instance()->CreateWorkerJob(fn);
	}

	void AutotestingSystem::OnScreenShotInternal(Image *image)
	{
		DVASSERT(image);
		
		Logger::Info("AutotestingSystem::OnScreenShot %s", screenShotName.c_str());
		uint64 startTime = SystemTimer::Instance()->AbsoluteMS();
		image->Save(FilePath(AutotestingDB::Instance()->logsFolder + Format("/%s.png", screenShotName.c_str())));
		uint64 finishTime = SystemTimer::Instance()->AbsoluteMS();
		Logger::FrameworkDebug("AutotestingSystem::OnScreenShot Upload: %d", finishTime - startTime);

		SafeRelease(image);
	}

	void AutotestingSystem::OnTestsFinished()
	{
		Logger::Info("AutotestingSystem::OnTestsFinished");

		// Mark last step as SUCCESS
		OnStepFinished();

		if (deviceId != "not-initialized")
		{
			AutotestingDB::Instance()->WriteState(deviceId, "finished");
		}

		// Mark test as SUCCESS
		AutotestingDB::Instance()->Log("INFO", "Test finished.");

		ExitApp();
	}

	void AutotestingSystem::OnInput(const UIEvent &input)
	{
		if (UIScreenManager::Instance())
		{
			String screenName = (UIScreenManager::Instance()->GetScreen()) ? UIScreenManager::Instance()->GetScreen()->GetName() : "noname";
			Logger::Info("AutotestingSystem::OnInput screen is %s (%d)", screenName.c_str(), UIScreenManager::Instance()->GetScreenId());
		}

		int32 id = input.tid;
		switch (input.phase)
		{
        case UIEvent::Phase::BEGAN:
        {
            mouseMove = input;
            if (!IsTouchDown(id))
            {
                touches[id] = input;
            }
			else
			{
                Logger::Error("AutotestingSystemYaml::OnInput PHASE_BEGAN duplicate touch id=%d", id);
            }
        }
        break;
#if !defined(__DAVAENGINE_IPHONE__) && !defined(__DAVAENGINE_ANDROID__)
        case UIEvent::Phase::MOVE:
        {
            mouseMove = input;
            if (IsTouchDown(id))
            {
                Logger::Error("AutotestingSystemYaml::OnInput PHASE_MOVE id=%d must be PHASE_DRAG", id);
            }
        }
        break;
#endif
        case UIEvent::Phase::DRAG:
        {
            mouseMove = input;
            Map<int32, UIEvent>::iterator findIt = touches.find(id);
            if (findIt != touches.end())
            {
                findIt->second = input;
			}
			else
			{
                Logger::Error("AutotestingSystemYaml::OnInput PHASE_DRAG id=%d must be PHASE_MOVE", id);
            }
        }
        break;
        case UIEvent::Phase::ENDED:
        {
            mouseMove = input;
            Map<int32, UIEvent>::iterator findIt = touches.find(id);
            if (findIt != touches.end())
            {
                touches.erase(findIt);
			}
			else
			{
                Logger::Error("AutotestingSystemYaml::OnInput PHASE_ENDED id=%d not found", id);
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
		if (findIt != touches.end())
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
		if (needExitApp)
		{
			return;
		}
		isRunning = false;
		isWaiting = false;
		needExitApp = true;
		timeBeforeExit = 1.0f;
	}

	// Multiplayer API


	// Working with DB api

};

#endif //__DAVAENGINE_AUTOTESTING__
