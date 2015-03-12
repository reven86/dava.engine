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
#include "Autotesting/AutotestingDB.h"

#ifdef __DAVAENGINE_AUTOTESTING__
#include "Platform/DeviceInfo.h"


// Work with MongoDb API
#define AUTOTESTING_TESTS "Tests"
#define AUTOTESTING_STEPS "Steps"
#define AUTOTESTING_LOG "Log"

namespace DAVA
{
	AutotestingDB::AutotestingDB()
	:dbClient(NULL)
	{

	}

	AutotestingDB::~AutotestingDB()
	{
		CloseConnection();
	}

	bool AutotestingDB::ConnectToDB(const String &collection, const String &dbName, const String &dbHost, const int32 dbPort)
	{
		DVASSERT(NULL == dbClient);

		dbClient = MongodbClient::Create(dbHost, dbPort);
		if(dbClient)
		{
			dbClient->SetDatabaseName(dbName);
			dbClient->SetCollectionName(collection);
		}

		return (NULL != dbClient);
	}

	void AutotestingDB::CloseConnection()
	{
		if(dbClient)
		{
			dbClient->Disconnect();
			SafeRelease(dbClient);
		}
	}

	String AutotestingDB::GetStringTestParameter(const String & deviceName, const String & parameter)
	{
		Logger::Debug("AutotestingDB::GetStringTestParameter deviceName=%s, parameter=%s", deviceName.c_str(), parameter.c_str());

		MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
		KeyedArchive* currentRunArchive = FindBuildArchive(dbUpdateObject, "autotesting_system");
		KeyedArchive* deviceArchive;
		String result;
		
		deviceArchive = currentRunArchive->GetArchive(deviceName);
		if (deviceArchive)
		{
			result = deviceArchive->GetString(parameter, "not_found");
		}
		else
		{
			AutotestingSystem::Instance()->ForceQuit(Format("Couldn't find archive for %s device", deviceName.c_str()));
		}
		

		SafeRelease(dbUpdateObject);
		Logger::Info("AutotestingDB::GetStringTestParameter %s", result.c_str());
		return result;
	}

	int32 AutotestingDB::GetIntTestParameter(const String & deviceName, const String & parameter)
	{
		Logger::Info("AutotestingDB::GetIntTestParameter deviceName=%s, parameter=%s", deviceName.c_str(), parameter.c_str());

		MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
		KeyedArchive* currentRunArchive = FindBuildArchive(dbUpdateObject, "autotesting_system");
		KeyedArchive* deviceArchive;
		int32 result;

		deviceArchive = currentRunArchive->GetArchive(deviceName);
		if (deviceArchive)
		{
			result = deviceArchive->GetInt32(parameter, -9999);
		}
		else
		{
			result = -9999;
		}


		SafeRelease(dbUpdateObject);
		Logger::Info("AutotestingDB::GetIntTestParameter %d", result);
		return result;
	}
	
	/*
	Design for AutotestDB:
	Doc for current build: id = Date_BuildNum_Device
		Doc for current test group: id = GroupName
			Doc for current test: id = TestNum
	*/

	// BUILD Level
	KeyedArchive *AutotestingDB::FindBuildArchive(MongodbUpdateObject* dbUpdateObject, const String &auxArg)
	{
		String testsName;

		if (auxArg.length() != 0)
		{
			testsName = Format("%s", auxArg.c_str());
		}
		else
		{
			testsName = Format("%s_%s_%s", AutotestingSystem::Instance()->buildDate.c_str(), AutotestingSystem::Instance()->buildId.c_str(), AutotestingSystem::Instance()->deviceName.c_str());
		}

		KeyedArchive* dbUpdateData = NULL;
		bool isFound = dbClient->FindObjectByKey(testsName, dbUpdateObject);
		
		if(!isFound)
		{
			AutotestingSystem::Instance()->ForceQuit(Format("AutotestingDB::FindRunArchive couldn't find %s archive", testsName.c_str()));
		}
		else
		{
			dbUpdateObject->LoadData();
			dbUpdateData = dbUpdateObject->GetData();
		}

		return dbUpdateData;
	}

	KeyedArchive *AutotestingDB::FindOrInsertBuildArchive(MongodbUpdateObject* dbUpdateObject, const String &auxArg)
	{
		String testsName;

		if (auxArg.length() != 0)
		{
			testsName = Format("%s", auxArg.c_str());
		}
		else
		{
			testsName = Format("%s_%s_%s", AutotestingSystem::Instance()->buildDate.c_str(), AutotestingSystem::Instance()->buildId.c_str(), AutotestingSystem::Instance()->deviceName.c_str());
		}

		bool isFound = dbClient->FindObjectByKey(testsName, dbUpdateObject);

		if(!isFound)
		{
			dbUpdateObject->SetObjectName(testsName);

			dbUpdateObject->AddString("Platform", DeviceInfo::GetPlatformString());
			dbUpdateObject->AddString("Date", AutotestingSystem::Instance()->buildDate);
			dbUpdateObject->AddString("RunId", AutotestingSystem::Instance()->runId);
			dbUpdateObject->AddString("Device", AutotestingSystem::Instance()->deviceName);			
			dbUpdateObject->AddString("BuildId", AutotestingSystem::Instance()->buildId);
			dbUpdateObject->AddString("Branch", AutotestingSystem::Instance()->branch);
			dbUpdateObject->AddString("BranchRevision", AutotestingSystem::Instance()->branchRev);
			dbUpdateObject->AddString("Framework", AutotestingSystem::Instance()->framework);
			dbUpdateObject->AddString("FrameworkRevision", AutotestingSystem::Instance()->frameworkRev);
			// TODO: After realization GetOsVersion() DF-3940
			dbUpdateObject->AddString("OSVersion", DeviceInfo::GetVersion());
			dbUpdateObject->AddString("Model", DeviceInfo::GetModel());

			Logger::Info("AutotestingSystem::InsertTestArchive new MongodbUpdateObject %s", testsName.c_str());
		}

		dbUpdateObject->LoadData();
		KeyedArchive* dbUpdateData = dbUpdateObject->GetData();

		return dbUpdateData;
	}

	void AutotestingDB::Log(const String &level, const String &message)
	{
		Logger::Info("AutotestingDB::Log [%s]%s", level.c_str(), message.c_str());
		uint64 startTime = SystemTimer::Instance()->AbsoluteMS();
		String testId = AutotestingSystem::Instance()->GetTestId();
		String stepId = AutotestingSystem::Instance()->GetStepId();
		String logId = AutotestingSystem::Instance()->GetLogId();
		AutotestingSystem::Instance()->logIndex++;

		MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();

		// Launch data
		dbUpdateObject->SetUniqueObjectName();

		dbUpdateObject->AddString("Platform", DeviceInfo::GetPlatformString());
		dbUpdateObject->AddString("Date", AutotestingSystem::Instance()->buildDate);
		dbUpdateObject->AddString("RunId", AutotestingSystem::Instance()->runId);
		dbUpdateObject->AddString("Device", AutotestingSystem::Instance()->deviceName);			
		dbUpdateObject->AddString("BuildId", AutotestingSystem::Instance()->buildId);
		dbUpdateObject->AddString("Branch", AutotestingSystem::Instance()->branch);
		dbUpdateObject->AddString("BranchRevision", AutotestingSystem::Instance()->branchRev);
		dbUpdateObject->AddString("Framework", AutotestingSystem::Instance()->framework);
		dbUpdateObject->AddString("FrameworkRevision", AutotestingSystem::Instance()->frameworkRev);
		dbUpdateObject->AddString("OSVersion", DeviceInfo::GetVersion());
		dbUpdateObject->AddString("Model", DeviceInfo::GetModel());

		// Test data
		dbUpdateObject->AddString("Group", AutotestingSystem::Instance()->groupName);
		dbUpdateObject->AddString("Test", testId);
		dbUpdateObject->AddString("Step", stepId);
		dbUpdateObject->AddString("Log", logId);


		// Log data
		String currentTime = AutotestingSystem::Instance()->GetCurrentTimeString();
		dbUpdateObject->AddString("Type", level);
		dbUpdateObject->AddString("Time", currentTime);
		dbUpdateObject->AddString("Message", message);
		/*KeyedArchive* logEntry = new KeyedArchive();

		logEntry->SetString("Type", level);
		
		logEntry->SetString("Time", currentTime);
		logEntry->SetString("Message", message);
		dbUpdateObject->Add*/

		dbUpdateObject->LoadData();
		SaveToDB(dbUpdateObject);
		SafeRelease(dbUpdateObject);
		
		uint64 finishTime = SystemTimer::Instance()->AbsoluteMS();
		Logger::Info("AutotestingSystem::Log FINISH  summary time %d", finishTime - startTime);
	}

	// DEPRECATED: Rewrite for new DB conception
	String AutotestingDB::ReadCommand(const String & device)
	{
		Logger::Info("AutotestingDB::ReadCommand device=%s", device.c_str());

		MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
		KeyedArchive* currentRunArchive = FindOrInsertBuildArchive(dbUpdateObject, "_multiplayer");

		String result;
		result = currentRunArchive->GetString(device + "_command", "not_found");

		SafeRelease(dbUpdateObject);

		Logger::Info("AutotestingDB::ReadCommand device=%s: '%s'", device.c_str(), result.c_str());
		return result;
	}

	// DEPRECATED: Rewrite for new DB conception
	String AutotestingDB::ReadState(const String & device)
	{
		Logger::Info("AutotestingDB::ReadState device=%s", device.c_str());


		MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
		KeyedArchive* currentRunArchive = FindOrInsertBuildArchive(dbUpdateObject, "_multiplayer");
		String result;

		result = currentRunArchive->GetString(device, "not_found");
		SafeRelease(dbUpdateObject);
		Logger::Info("AutotestingDB::ReadState device=%s: '%s'", device.c_str(), result.c_str());
		return result;
	}

	// DEPRECATED: Rewrite for new DB conception
	String AutotestingDB::ReadString(const String & name)
	{
		Logger::Info("AutotestingSystem::ReadString name=%s", name.c_str());

		MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
		KeyedArchive* currentRunArchive = FindOrInsertBuildArchive(dbUpdateObject, "_aux");
		String result;

		result = currentRunArchive->GetString(name, "not_found");

		SafeRelease(dbUpdateObject);
		Logger::Info("AutotestingSystem::ReadString name=%name: '%s'", name.c_str(), result.c_str());
		return result;
	}

	bool AutotestingDB::SaveKeyedArchiveToDB(const String &archiveName, KeyedArchive *archive, const String &docName)
	{
		Logger::Info("AutotestingDB::SaveKeyedArchiveToDB %s to %s", archiveName.c_str(), docName.c_str());
		String testId = AutotestingSystem::Instance()->GetTestId();
		String stepId = AutotestingSystem::Instance()->GetStepId();
		
		MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();

		// Launch data
		dbUpdateObject->SetUniqueObjectName();

		dbUpdateObject->AddString("Platform", DeviceInfo::GetPlatformString());
		dbUpdateObject->AddString("Date", AutotestingSystem::Instance()->buildDate);
		dbUpdateObject->AddString("Device", AutotestingSystem::Instance()->deviceName);			
		dbUpdateObject->AddString("BuildId", AutotestingSystem::Instance()->buildId);
		dbUpdateObject->AddString("Branch", AutotestingSystem::Instance()->branch);
		dbUpdateObject->AddString("BranchRevision", AutotestingSystem::Instance()->branchRev);
		dbUpdateObject->AddString("Framework", AutotestingSystem::Instance()->framework);
		dbUpdateObject->AddString("FrameworkRevision", AutotestingSystem::Instance()->frameworkRev);
		dbUpdateObject->AddString("OSVersion", DeviceInfo::GetVersion());
		dbUpdateObject->AddString("Model", DeviceInfo::GetModel());

		// Test data
		dbUpdateObject->AddString("Group", AutotestingSystem::Instance()->groupName);
		dbUpdateObject->AddString("Test", testId);
		dbUpdateObject->AddString("Step", stepId);


		// Insert Archive
		dbUpdateObject->AddString("Type", docName);
		dbUpdateObject->AddString("Name", archiveName);
		dbUpdateObject->LoadData();
		dbUpdateObject->GetData()->SetArchive(archiveName, archive);
		

		return SaveToDB(dbUpdateObject);;
	}

	bool AutotestingDB::SaveToDB(MongodbUpdateObject *dbUpdateObject)
	{
		uint64 startTime = SystemTimer::Instance()->AbsoluteMS();
		Logger::Info("AutotestingSystem::SaveToDB");

		bool ret = dbUpdateObject->SaveToDB(dbClient);

		if(!ret)
		{
			Logger::Error("AutotestingSystem::SaveToDB failed");
		}

		uint64 finishTime = SystemTimer::Instance()->AbsoluteMS();
		Logger::Info("AutotestingSystem::SaveToDB FINISH result time %d", finishTime - startTime);
		return ret;
	}

	void AutotestingDB::UploadScreenshot(const String & name, Image *image)
	{
		//#if defined(__DAVAENGINE_ANDROID__)
		//	image->ResizeImage(1280, 752);
		//	image->ResizeCanvas(1280, 752);
		//#else
		//	image->ResizeImage(1024, 768);
		//	image->ResizeCanvas(1024, 768);
		//#endif
		if(dbClient)
		{
			//Logger::Info("Image: datasize %d, %d x %d", image->dataSize, image->GetHeight(), image->GetWidth());
            dbClient->SaveBufferToGridFS(Format("%s_%dx%d", name.c_str(), image->GetWidth(), image->GetHeight()),
                reinterpret_cast<char*>(image->GetData()), image->dataSize);
		}
	}

	// DEPRECATED: Rewrite for new DB conception
	void AutotestingDB::WriteCommand(const String & device, const String & command)
	{
		Logger::Info("AutotestingDB::WriteCommand device=%s command=%s", device.c_str(), command.c_str());

		MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
		KeyedArchive* currentRunArchive = FindOrInsertBuildArchive(dbUpdateObject, "_multiplayer");

		currentRunArchive->SetString(device + "_command", command);

		SaveToDB(dbUpdateObject);
		SafeRelease(dbUpdateObject);
		//Logger::Info("AutotestingSystem::WriteCommand finish");
	}

	// DEPRECATED: Rewrite for new DB conception
	void AutotestingDB::WriteState(const String & device, const String & state)
	{
		Logger::Info("AutotestingDB::WriteState device=%s state=%s", device.c_str(), state.c_str());

		MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
		KeyedArchive* currentRunArchive = FindOrInsertBuildArchive(dbUpdateObject, "_multiplayer");

		currentRunArchive->SetString(device, state);

		//SafeRelease(multiplayerArchive);
		SaveToDB(dbUpdateObject);
		SafeRelease(dbUpdateObject);

		//Logger::Info("AutotestingSystem::WriteState finish");
	}

	// DEPRECATED: Rewrite for new DB conception
	void AutotestingDB::WriteString(const String & name, const String & text)
	{
		Logger::Info("AutotestingSystem::WriteString name=%s text=%s", name.c_str(), text.c_str());

		MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
		KeyedArchive* currentRunArchive = FindOrInsertBuildArchive(dbUpdateObject, "_aux");

		currentRunArchive->SetString(name, text);

		SaveToDB(dbUpdateObject);
		SafeRelease(dbUpdateObject);

		Logger::Info("AutotestingSystem::WriteString finish");
	}
	
	// auxiliary methods
	void AutotestingDB::SetTestStarted()
	{
		Logger::Info("AutotestingSystem::SetTestStarted Test%03d: %s", AutotestingSystem::Instance()->testIndex, AutotestingSystem::Instance()->testName.c_str());

		MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
		KeyedArchive* currentRunArchive = FindOrInsertBuildArchive(dbUpdateObject, "autotesting_system");
		
		KeyedArchive* deviceArchive = currentRunArchive->GetArchive(AutotestingSystem::Instance()->deviceName, NULL);
		
		if (deviceArchive)
		{
			deviceArchive->SetString("Started", "1");
			deviceArchive->SetString("BuildId", AutotestingSystem::Instance()->buildId);
			deviceArchive->SetString("Date", AutotestingSystem::Instance()->testsDate);
			deviceArchive->SetString("Framework", AutotestingSystem::Instance()->framework);
			deviceArchive->SetString("Branch", AutotestingSystem::Instance()->branch);
		}
		else
		{
			AutotestingSystem::Instance()->ForceQuit(Format("Couldn't find archive for %s device", AutotestingSystem::Instance()->deviceName.c_str()));
		}
		SaveToDB(dbUpdateObject);
		SafeRelease(dbUpdateObject);
	}
}

#endif //__DAVAENGINE_AUTOTESTING__