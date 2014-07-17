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

		dbClient = MongodbClient::Create(dbHost.c_str(), dbPort);
		if(dbClient)
		{
			dbClient->SetDatabaseName(dbName.c_str());
			dbClient->SetCollectionName(collection.c_str());
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
		
		deviceArchive = currentRunArchive->GetArchive(deviceName.c_str());
		if (deviceArchive)
		{
			result = deviceArchive->GetString(parameter.c_str(), "not_found");
		}
		else
		{
			AutotestingSystem::Instance()->ForceQuit(Format("Couldn't find archive for %s device", deviceName.c_str()));
		}
		

		SafeRelease(dbUpdateObject);
		Logger::Debug("AutotestingDB::GetStringTestParameter %s", result.c_str());
		return result;
	}

	int32 AutotestingDB::GetIntTestParameter(const String & deviceName, const String & parameter)
	{
		Logger::Debug("AutotestingDB::GetIntTestParameter deviceName=%s, parameter=%s", deviceName.c_str(), parameter.c_str());

		MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
		KeyedArchive* currentRunArchive = FindBuildArchive(dbUpdateObject, "autotesting_system");
		KeyedArchive* deviceArchive;
		int32 result;

		deviceArchive = currentRunArchive->GetArchive(deviceName.c_str());
		if (deviceArchive)
		{
			result = deviceArchive->GetInt32(parameter.c_str(), -9999);
		}
		else
		{
			result = -9999;
		}


		SafeRelease(dbUpdateObject);
		Logger::Debug("AutotestingDB::GetIntTestParameter %d", result);
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

		KeyedArchive* dbUpdateData;
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

		KeyedArchive* dbUpdateData;
		bool isFound = dbClient->FindObjectByKey(testsName, dbUpdateObject);

		if(!isFound)
		{
			dbUpdateObject->SetObjectName(testsName);

			dbUpdateObject->AddString("Platform", AUTOTESTING_PLATFORM_NAME);
			dbUpdateObject->AddString("Date", AutotestingSystem::Instance()->buildDate.c_str());
			dbUpdateObject->AddString("Device", AutotestingSystem::Instance()->deviceName.c_str());			
			dbUpdateObject->AddString("BuildId", AutotestingSystem::Instance()->buildId.c_str());
			dbUpdateObject->AddString("Branch", AutotestingSystem::Instance()->branch.c_str());
			dbUpdateObject->AddString("BranchRevision", AutotestingSystem::Instance()->branchRev.c_str());
			dbUpdateObject->AddString("Framework", AutotestingSystem::Instance()->framework.c_str());
			dbUpdateObject->AddString("FramewrokRevision", AutotestingSystem::Instance()->frameworkRev.c_str());
			// TODO: After realization GetOsVersion() DF-3940
			dbUpdateObject->AddString("OSVersion", DeviceInfo::GetVersion());
			dbUpdateObject->AddString("Model", DeviceInfo::GetModel());

			Logger::Debug("AutotestingSystem::InsertTestArchive new MongodbUpdateObject %s", testsName.c_str());
		}

		dbUpdateObject->LoadData();
		dbUpdateData = dbUpdateObject->GetData();

		return dbUpdateData;
	}


	// GROUP Level
	KeyedArchive *AutotestingDB::FindOrInsertGroupArchive(KeyedArchive* buildArchive, const String &groupId)
	{
		Logger::Debug("AutotestingDB::FindInsertGroupArchive Group=%s", groupId.c_str());

		KeyedArchive* currentGroupArchive = NULL;
		currentGroupArchive = SafeRetain(buildArchive->GetArchive(groupId, NULL));

		if(!currentGroupArchive)
		{
			currentGroupArchive = new KeyedArchive();

			currentGroupArchive->SetString("Name", groupId);
			currentGroupArchive->SetString("Date", AutotestingSystem::Instance()->testsDate);

			buildArchive->SetArchive(groupId, currentGroupArchive);
			Logger::Debug("AutotestingDB::FindInsertGroupArchive new Group=%s", groupId.c_str());
		}
		SafeRelease(currentGroupArchive);
		
		currentGroupArchive = buildArchive->GetArchive(groupId);

		return currentGroupArchive;
	}


	// TEST Level
	KeyedArchive *AutotestingDB::FindOrInsertTestArchive(MongodbUpdateObject* dbUpdateObject, const String &testId)
	{
		Logger::Debug("AutotestingDB::FindOrInsertTestArchive testId=%s", testId.c_str());

		KeyedArchive* dbUpdateData = FindOrInsertBuildArchive(dbUpdateObject, "");

		KeyedArchive* currentGroupArchive = dbUpdateData->GetArchive(AutotestingSystem::Instance()->groupName, NULL);
		
		if (! currentGroupArchive)
		{
			AutotestingSystem::Instance()->ForceQuit(Format("Couldn't find archive for %s device", AutotestingSystem::Instance()->groupName.c_str()));
		}

		KeyedArchive* currentTestArchive = SafeRetain(currentGroupArchive->GetArchive(testId, NULL));

		if(!currentTestArchive)
		{
			currentTestArchive = new KeyedArchive();

			currentTestArchive->SetString("Name", AutotestingSystem::Instance()->testName);
			currentTestArchive->SetString("FileName", AutotestingSystem::Instance()->testFileName);

			currentGroupArchive->SetArchive(testId, currentTestArchive);
			Logger::Debug("AutotestingDB::FindOrInsertTestArchive new %s", testId.c_str());
		}
		SafeRelease(currentTestArchive);
		currentTestArchive = currentGroupArchive->GetArchive(testId);

		return currentTestArchive;
	}

	KeyedArchive *AutotestingDB::InsertTestArchive(KeyedArchive* currentGroupArchive, const String &testId)
	{
		Logger::Debug("AutotestingDB::InsertTestArchive testId=%s", testId.c_str());
		KeyedArchive* currentTestArchive = new KeyedArchive();

		currentTestArchive->SetString("FileName", AutotestingSystem::Instance()->testFileName);
		currentTestArchive->SetBool("Success", false);

		KeyedArchive* stepsArchive = new KeyedArchive();
		currentTestArchive->SetArchive(AUTOTESTING_STEPS, stepsArchive);
		SafeRelease(stepsArchive);

		currentGroupArchive->SetArchive(testId, currentTestArchive);
		Logger::Debug("AutotestingDB::InsertTestArchive new %s", testId.c_str());

		SafeRelease(currentTestArchive);
		currentTestArchive = currentGroupArchive->GetArchive(testId);

		return currentTestArchive;
	}

	
	// STEP Level
	KeyedArchive *AutotestingDB::FindOrInsertStepArchive(KeyedArchive *testArchive, const String &stepId)
	{
		Logger::Debug("AutotestingDB::FindOrInsertTestStepArchive stepId=%s", stepId.c_str());

		KeyedArchive* currentTestStepArchive = NULL;
		KeyedArchive* testStepsArchive = NULL;

		testStepsArchive = SafeRetain(testArchive->GetArchive(AUTOTESTING_STEPS, NULL));
		if(testStepsArchive)
		{
			currentTestStepArchive = SafeRetain(testStepsArchive->GetArchive(stepId, NULL));
		}

		if(!currentTestStepArchive)
		{
			currentTestStepArchive = new KeyedArchive();
			if(!testStepsArchive)
			{
				testStepsArchive = new KeyedArchive();
				testArchive->SetArchive(AUTOTESTING_STEPS, testStepsArchive);
				SafeRelease(testStepsArchive);
				testStepsArchive = SafeRetain(testArchive->GetArchive(AUTOTESTING_STEPS));
			}
			testStepsArchive->SetArchive(stepId, currentTestStepArchive);
			Logger::Debug("AutotestingDB::FindOrInsertTestStepArchive new %s", stepId.c_str());
		}
		SafeRelease(currentTestStepArchive);
		currentTestStepArchive = testStepsArchive->GetArchive(stepId);
		SafeRelease(testStepsArchive);

		//Logger::Debug("AutotestingSystem::FindOrInsertTestStepArchive finish");
		return currentTestStepArchive;
	}

	KeyedArchive *AutotestingDB::InsertStepArchive(KeyedArchive *testArchive, const String &stepId, const String &description)
	{
		Logger::Debug("AutotestingDB::InsertStepArchive stepId=%s description=%s", stepId.c_str(), description.c_str());

		KeyedArchive* currentStepArchive = NULL;
		KeyedArchive* testStepsArchive = NULL;

		testStepsArchive = SafeRetain(testArchive->GetArchive(AUTOTESTING_STEPS, NULL));
		if(!testStepsArchive)
		{
			testStepsArchive = new KeyedArchive();
			testArchive->SetArchive(AUTOTESTING_STEPS, testStepsArchive);
			SafeRelease(testStepsArchive);
			testStepsArchive = SafeRetain(testArchive->GetArchive(AUTOTESTING_STEPS));
		}

		currentStepArchive = new KeyedArchive();
		currentStepArchive->SetString("Description", description.c_str());
		currentStepArchive->SetBool("Success", false);

		KeyedArchive* logArchive = new KeyedArchive();
		currentStepArchive->SetArchive(AUTOTESTING_LOG, logArchive);
		SafeRelease(logArchive);

		testStepsArchive->SetArchive(stepId, currentStepArchive);
		Logger::Debug("AutotestingDB::InsertStepArchive new %s", stepId.c_str());

		SafeRelease(currentStepArchive);
		currentStepArchive = testStepsArchive->GetArchive(stepId);
		SafeRelease(testStepsArchive);

		return currentStepArchive;
	}
	
	KeyedArchive *AutotestingDB::FindOrInsertTestStepLogEntryArchive(KeyedArchive *testStepArchive, const String &logId)
	{
		Logger::Debug("AutotestingDB::FindOrInsertTestStepLogEntryArchive logId=%s", logId.c_str());

		KeyedArchive* currentTestStepLogEntryArchive = NULL;
		KeyedArchive* testStepLogArchive = NULL;

		testStepLogArchive = SafeRetain(testStepArchive->GetArchive(AUTOTESTING_LOG, NULL));
		if(testStepLogArchive)
		{
			currentTestStepLogEntryArchive = SafeRetain(testStepLogArchive->GetArchive(logId, NULL));
		}

		if(!currentTestStepLogEntryArchive)
		{
			currentTestStepLogEntryArchive = new KeyedArchive();
			if(!testStepLogArchive)
			{
				testStepLogArchive = new KeyedArchive();
				testStepArchive->SetArchive(AUTOTESTING_LOG, testStepLogArchive);
				SafeRelease(testStepLogArchive);
				testStepLogArchive = SafeRetain(testStepArchive->GetArchive(AUTOTESTING_LOG));
			}
			testStepLogArchive->SetArchive(logId, currentTestStepLogEntryArchive);
		}
		SafeRelease(currentTestStepLogEntryArchive);
		currentTestStepLogEntryArchive = testStepLogArchive->GetArchive(logId);
		SafeRelease(testStepLogArchive);

		//Logger::Debug("AutotestingSystem::FindOrInsertTestStepLogEntryArchive finish");
		return currentTestStepLogEntryArchive;
	}


	void AutotestingDB::Log(const String &level, const String &message)
	{
		Logger::Debug("AutotestingDB::Log [%s]%s", level.c_str(), message.c_str());
		uint64 startTime = SystemTimer::Instance()->AbsoluteMS();
		String testId = AutotestingSystem::Instance()->GetTestId();
		String stepId = AutotestingSystem::Instance()->GetStepId();
		String logId = AutotestingSystem::Instance()->GetLogId();
		AutotestingSystem::Instance()->logIndex++;

		MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
		KeyedArchive* currentTestArchive = FindOrInsertTestArchive(dbUpdateObject, testId);
		KeyedArchive* currentStepArchive = FindOrInsertStepArchive(currentTestArchive, stepId); 

		KeyedArchive* logEntry = FindOrInsertTestStepLogEntryArchive(currentStepArchive, logId);

		logEntry->SetString("Type", level);
		String currentTime = AutotestingSystem::Instance()->GetCurrentTimeString();
		logEntry->SetString("Time", currentTime);
		logEntry->SetString("Message", message);

		SaveToDB(dbUpdateObject);
		SafeRelease(dbUpdateObject);
		
		uint64 finishTime = SystemTimer::Instance()->AbsoluteMS();
		Logger::Debug("AutotestingSystem::Log FINISH  summary time %d", finishTime - startTime);
	}

	String AutotestingDB::ReadCommand(const String & device)
	{
		Logger::Debug("AutotestingDB::ReadCommand device=%s", device.c_str());

		MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
		KeyedArchive* currentRunArchive = FindOrInsertBuildArchive(dbUpdateObject, "_multiplayer");

		String result;
		result = currentRunArchive->GetString(device + "_command", "not_found");

		SafeRelease(dbUpdateObject);

		Logger::Debug("AutotestingDB::ReadCommand device=%s: '%s'", device.c_str(), result.c_str());
		return result;
	}

	String AutotestingDB::ReadState(const String & device)
	{
		Logger::Debug("AutotestingDB::ReadState device=%s", device.c_str());


		MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
		KeyedArchive* currentRunArchive = FindOrInsertBuildArchive(dbUpdateObject, "_multiplayer");
		String result;

		result = currentRunArchive->GetString(device.c_str(), "not_found");
		SafeRelease(dbUpdateObject);
		Logger::Debug("AutotestingDB::ReadState device=%s: '%s'", device.c_str(), result.c_str());
		return result;
	}

	String AutotestingDB::ReadString(const String & name)
	{
		Logger::Debug("AutotestingSystem::ReadString name=%s", name.c_str());

		MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
		KeyedArchive* currentRunArchive = FindOrInsertBuildArchive(dbUpdateObject, "_aux");
		String result;

		result = currentRunArchive->GetString(name.c_str(), "not_found");

		SafeRelease(dbUpdateObject);
		Logger::Debug("AutotestingSystem::ReadString name=%name: '%s'", name.c_str(), result.c_str());
		return result;
	}

	bool AutotestingDB::SaveKeyedArchiveToDB(const String &archiveName, KeyedArchive *archive, const String &docName)
	{
		bool ret = false;
		MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();

		KeyedArchive* currentBuildArchive = AutotestingDB::Instance()->FindOrInsertBuildArchive(dbUpdateObject, "");
		KeyedArchive* currentGroupArchive = AutotestingDB::Instance()->FindOrInsertGroupArchive(currentBuildArchive, AutotestingSystem::Instance()->groupName);

		currentGroupArchive->SetArchive(archiveName.c_str(), archive);
		currentGroupArchive->SetString("Map", docName.c_str());
		ret = SaveToDB(dbUpdateObject);
		//SafeRelease(currentBuildArchive);
		//SafeRelease(currentGroupArchive);
		SafeRelease(dbUpdateObject);
		return ret;
	}

	bool AutotestingDB::SaveToDB(MongodbUpdateObject *dbUpdateObject)
	{
		uint64 startTime = SystemTimer::Instance()->AbsoluteMS();
		Logger::Debug("AutotestingSystem::SaveToDB");

		bool ret = dbUpdateObject->SaveToDB(dbClient);

		if(!ret)
		{
			Logger::Error("AutotestingSystem::SaveToDB failed");
		}

		uint64 finishTime = SystemTimer::Instance()->AbsoluteMS();
		Logger::Debug("AutotestingSystem::SaveToDB FINISH result time %d", finishTime - startTime);
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
			//Logger::Debug("Image: datasize %d, %d x %d", image->dataSize, image->GetHeight(), image->GetWidth());
            dbClient->SaveBufferToGridFS(Format("%s_%dx%d", name.c_str(), image->GetWidth(), image->GetHeight()),
                reinterpret_cast<char*>(image->GetData()), image->dataSize);
		}
	}

	void AutotestingDB::WriteCommand(const String & device, const String & command)
	{
		Logger::Debug("AutotestingDB::WriteCommand device=%s command=%s", device.c_str(), command.c_str());

		MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
		KeyedArchive* currentRunArchive = FindOrInsertBuildArchive(dbUpdateObject, "_multiplayer");

		currentRunArchive->SetString(device + "_command", command);

		SaveToDB(dbUpdateObject);
		SafeRelease(dbUpdateObject);
		//Logger::Debug("AutotestingSystem::WriteCommand finish");
	}

	void AutotestingDB::WriteState(const String & device, const String & state)
	{
		Logger::Debug("AutotestingDB::WriteState device=%s state=%s", device.c_str(), state.c_str());

		MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
		KeyedArchive* currentRunArchive = FindOrInsertBuildArchive(dbUpdateObject, "_multiplayer");

		currentRunArchive->SetString(device, state);

		//SafeRelease(multiplayerArchive);
		SaveToDB(dbUpdateObject);
		SafeRelease(dbUpdateObject);

		//Logger::Debug("AutotestingSystem::WriteState finish");
	}

	void AutotestingDB::WriteString(const String & name, const String & text)
	{
		Logger::Debug("AutotestingSystem::WriteString name=%s text=%s", name.c_str(), text.c_str());

		MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
		KeyedArchive* currentRunArchive = FindOrInsertBuildArchive(dbUpdateObject, "_aux");

		currentRunArchive->SetString(name, text);

		SaveToDB(dbUpdateObject);
		SafeRelease(dbUpdateObject);

		Logger::Debug("AutotestingSystem::WriteString finish");
	}
	
	// auxiliary methods
	void AutotestingDB::SetTestStarted()
	{
		Logger::Debug("AutotestingSystem::SetTestStarted Test%03d: %s", AutotestingSystem::Instance()->testIndex, AutotestingSystem::Instance()->testName.c_str());

		MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
		KeyedArchive* currentRunArchive = FindOrInsertBuildArchive(dbUpdateObject, "autotesting_system");
		
		KeyedArchive* deviceArchive = currentRunArchive->GetArchive(AutotestingSystem::Instance()->deviceName.c_str(), NULL);
		
		if (deviceArchive)
		{
			deviceArchive->SetString("Started", "1");
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