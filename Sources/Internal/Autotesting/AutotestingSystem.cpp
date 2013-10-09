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

#ifdef AUTOTESTING_LUA
#include "AutotestingSystemLua.h"
#else
#include "AutotestingSystemYaml.h"
#endif

namespace DAVA
{

AutotestingSystem::AutotestingSystem()
    : isInit(false)
    , isRunning(false)
    , needExitApp(false)
    , timeBeforeExit(0.0f)
    , testsId(0)
    , testsDate(0)
    , testIndex(0)
    , dbClient(NULL)
    , isDB(false)
    , needClearGroupInDB(false)
    , reportFile(NULL)
    , groupName("default")
	, device("default")
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
#ifdef AUTOTESTING_LUA
    new AutotestingSystemLua();
#else
    new AutotestingSystemYaml();
#endif
}

AutotestingSystem::~AutotestingSystem()
{
    AutotestingSystemLua::Instance()->Release();
}

void AutotestingSystem::OnAppStarted()
{
	if(!isInit)
    {
        // get files list for ~res:/Autotesting/Tests
        FileList fileList("~res:/Autotesting/Tests/");
        int32 fileListSize = fileList.GetCount();
        if(fileListSize == 0)
        {
            return;
        }
        
        // read current test index and autotesting id from ~doc:/autotesting.archive
        KeyedArchive* autotestingArchive = new KeyedArchive();
        
        if(autotestingArchive->Load("~doc:/autotesting/autotesting.archive"))
        {
            testIndex = autotestingArchive->GetInt32("index");
            stepIndex = 0;
            logIndex = 0;
            VariantType* vt = autotestingArchive->GetVariant("id");
            if(vt && vt->type == VariantType::TYPE_UINT32)
            {
                testsId = vt->AsUInt32();
            }
        }
        
        uint32 autotestingId = 1;
        
        //TODO: read autotesting id from ~res:/Autotesting/autotesting.archive
        File* file = File::Create("~res:/Autotesting/id.txt", File::OPEN | File::READ);
        if(file)
        {
            char tempBuf[1024];
            file->ReadLine(tempBuf, 1024);
            sscanf(tempBuf, "%d", &autotestingId);
            
            if(!file->IsEof())
            {
                char tempBuf[1024];
                file->ReadLine(tempBuf, 1024);
                sscanf(tempBuf, "%d", &testsDate);
            }
            
            if(!file->IsEof())
            {
                char projectCharName[128];
                file->ReadLine(tempBuf, 1024);
                sscanf(tempBuf, "%s", projectCharName);
                SetProjectName(projectCharName);
            }
            
            if(!file->IsEof())
            {
                char groupCharName[128];
                file->ReadLine(tempBuf, 1024);
                sscanf(tempBuf, "%s", groupCharName);
                String gName = groupCharName;
                if(!gName.empty())
                {
                    groupName = gName;
                }
            }

			if(!file->IsEof())
			{
				char deviceCharName[128];
				file->ReadLine(tempBuf, 1024);
				sscanf(tempBuf, "%s", deviceCharName);
				String dName = deviceCharName;
				if(!dName.empty())
				{
					device = dName;
				}
			}
            isDB = true;
        }
		else
		{
			// build for local testing
			isDB = false;
		}
        SafeRelease(file);
        
		if(isDB)
		{
			if(!ConnectToDB())
			{
				return;
			}
		}
        
        // compare ids
        if(testsId != autotestingId)
        {
            testIndex = 0;
            stepIndex = 0;
            logIndex = 0;
            testsId = autotestingId;
        }
        
        int32 indexInFileList = GetIndexInFileList(fileList, testIndex);
        
        // try to cycle
        if(fileListSize <= indexInFileList)
        {
            testIndex = 0;
            stepIndex = 0;
            logIndex = 0;
            indexInFileList = GetIndexInFileList(fileList, testIndex);
        }
        
        needClearGroupInDB = (testIndex == 0);
        
        if(indexInFileList < fileListSize)
        {
            // found direct or cycled
            
            testFilePath = fileList.GetPathname(indexInFileList).GetAbsolutePathname();
            testFileName = fileList.GetFilename(indexInFileList);
            testName = testFileName;
            
            // create folder for report
            testReportsFolder = "~doc:/autotesting/";
            FileSystem::Instance()->CreateDirectory(testReportsFolder, true);
            
            reportFile = File::Create(testReportsFolder + "autotesting.report", File::CREATE | File::WRITE);
            
            // Create document for test
            ClearTestInDB();
            
#ifdef AUTOTESTING_LUA
            AutotestingSystemLua::Instance()->InitFromFile(testFilePath);
#else
            AutotestingSystemYaml::Instance()->InitFromYaml(testFilePath);
#endif
            
            // save next index and autotesting id
            autotestingArchive->SetUInt32("id", testsId);
            autotestingArchive->SetInt32("index", (testIndex + 1));
        }
        else
        {
            // not found
            
            // last file (or after last) - reset id and index
            //autotestingArchive->SetUInt32("id", 0); //don't reset id - allow cycled tests
            autotestingArchive->SetInt32("index", 0);
        }
        autotestingArchive->Save("~doc:/autotesting/autotesting.archive");
        SafeRelease(autotestingArchive);
    }
}  
    
int32 AutotestingSystem::GetIndexInFileList(FileList &fileList, int32 index)
{
    int32 fileListSize = fileList.GetCount();
    int32 indexInFileList = index;
    // skip directories
    for(int32 i = 0; (i <= indexInFileList) && (i < fileListSize); ++i)
    {
        if((fileList.IsDirectory(i)) || (fileList.IsNavigationDirectory(i)))
        {
            indexInFileList++;
        }
        else
        {
            String fileExtension = fileList.GetPathname(i).GetExtension();
#ifdef AUTOTESTING_LUA
            //skip all non-lua files
            if(fileExtension != ".lua")
            {
                indexInFileList++;
            }
#else
            // skip all non-yaml files
            if(fileExtension != ".yaml")
            {
                indexInFileList++;
            }
#endif
        }
    }
    return indexInFileList;
}
    
void AutotestingSystem::OnAppFinished()
{
    if(dbClient)
    {
        dbClient->Disconnect();
        SafeRelease(dbClient);
    }
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
		Log("DEBUG", "OnInit");
    }
}
    
void AutotestingSystem::SetProjectName(const String &_projectName)
{
    projectName = _projectName;
}
    
bool AutotestingSystem::ConnectToDB()
{
    DVASSERT(NULL == dbClient);
    
    dbClient = MongodbClient::Create(AUTOTESTING_DB_HOST, AUTOTESTING_DB_PORT);
    if(dbClient)
    {
        dbClient->SetDatabaseName(AUTOTESTING_DB_NAME);
        dbClient->SetCollectionName(projectName);
    }
    
    return (NULL != dbClient);
}
    
//void AutotestingSystem::AddTestResult(const String &text, bool isPassed, const String &error)
//{
//    testResults.push_back(TestResult(text, isPassed, error));
//}
  

// Work with MongoDb API
#define AUTOTESTING_TESTS "Tests"
#define AUTOTESTING_STEPS "Steps"
#define AUTOTESTING_LOG "Log"

void AutotestingSystem::WriteString(const String & name, const String & text)
{
	Logger::FrameworkDebug("AutotestingSystem::WriteString name=%s text=%s", name.c_str(), text.c_str());

	MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
	KeyedArchive* currentRunArchive = FindOrInsertRunArchive(dbUpdateObject, "_aux");

	currentRunArchive->SetString(name, text);

	SaveToDB(dbUpdateObject);
	SafeRelease(dbUpdateObject);

	Logger::FrameworkDebug("AutotestingSystem::WriteString finish");
}

String AutotestingSystem::ReadString(const String & name)
{
	Logger::FrameworkDebug("AutotestingSystem::ReadString name=%s", name.c_str());
	
	MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
	KeyedArchive* currentRunArchive = FindOrInsertRunArchive(dbUpdateObject, "_aux");
	String result;

	result = currentRunArchive->GetString(name.c_str(), "not_found");

	SafeRelease(dbUpdateObject);
	Logger::FrameworkDebug("AutotestingSystem::ReadString name=%name: '%s'", name.c_str(), result.c_str());
	return result;
}

void AutotestingSystem::ClearTestInDB()
{
    MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
    
    // clear test
    String testId = GetTestId(testIndex);
    KeyedArchive* currentTestArchive = InsertTestArchive(dbUpdateObject, testId, testName, needClearGroupInDB);
    
    // Create document for step 0 -  'Precondition'
    String stepId = GetStepId(stepIndex);
    InsertStepArchive(currentTestArchive, stepId, "Precondition");

    SaveToDB(dbUpdateObject);
    SafeRelease(dbUpdateObject);
}

KeyedArchive *AutotestingSystem::FindOrInsertRunArchive(MongodbUpdateObject* dbUpdateObject, const String &auxArg)
{
	String testsName;

	if (auxArg.length() != 0)
	{
		testsName = Format("%u_%s", testsDate, auxArg.c_str());
	}
	else
	{
		testsName = Format("%u_%s_%s_%s", testsDate, AUTOTESTING_PLATFORM_NAME, device.c_str(), groupName.c_str());
	}

	KeyedArchive* dbUpdateData;
	bool isFound = dbClient->FindObjectByKey(testsName, dbUpdateObject);
	if(!isFound)
	{
		dbUpdateObject->SetObjectName(testsName);
		Logger::FrameworkDebug("AutotestingSystem::InsertTestArchive new MongodbUpdateObject %s", testsName.c_str());
		dbUpdateObject->LoadData();
		dbUpdateData = dbUpdateObject->GetData();

		dbUpdateData->SetString("Platform", AUTOTESTING_PLATFORM_NAME);
		dbUpdateData->SetString("Date", Format("%u", testsDate));
		dbUpdateData->SetString("Group", groupName);
		dbUpdateData->SetString("Device", device);

		//SaveToDB(dbUpdateObject);
	}
	else
	{
		dbUpdateObject->LoadData();
		dbUpdateData = dbUpdateObject->GetData();
	}
	
	return dbUpdateData;
}

bool AutotestingSystem::SaveKeyedArchiveToDB(const String &archiveName, KeyedArchive *archive, const String &docName)
{
	bool ret = false;
	MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
    
    KeyedArchive* dbUpdateData = FindOrInsertRunArchive(dbUpdateObject, docName);
	
	dbUpdateData->SetArchive(archiveName.c_str(), archive);
	dbUpdateData->SetString("Map", docName.c_str());

	ret = SaveToDB(dbUpdateObject);
    SafeRelease(dbUpdateObject);
	return ret;
}

KeyedArchive *AutotestingSystem::FindOrInsertTestArchive(MongodbUpdateObject* dbUpdateObject, const String &testId)
{
    Logger::FrameworkDebug("AutotestingSystem::FindOrInsertTestArchive testId=%s", testId.c_str());
    KeyedArchive* currentTestArchive = NULL;
    
    KeyedArchive* dbUpdateData = FindOrInsertRunArchive(dbUpdateObject, "");
    
	currentTestArchive = SafeRetain(dbUpdateData->GetArchive(testId, NULL));
	    
    if(!currentTestArchive)
    {
        currentTestArchive = new KeyedArchive();
        
        currentTestArchive->SetString("Name", testName);
        currentTestArchive->SetString("FileName", testFileName);
        
        dbUpdateData->SetArchive(testId, currentTestArchive);
        Logger::FrameworkDebug("AutotestingSystem::FindOrInsertTestArchive new %s", testId.c_str());
    }
    SafeRelease(currentTestArchive);
    currentTestArchive = dbUpdateData->GetArchive(testId);
    
	//Logger::FrameworkDebug("AutotestingSystem::FindOrInsertTestArchive finish");
    return currentTestArchive;
}
   
KeyedArchive *AutotestingSystem::InsertTestArchive(MongodbUpdateObject* dbUpdateObject, const String &testId, const String &testName, bool needClearGroup)
{
	Logger::FrameworkDebug("AutotestingSystem::InsertTestArchive testId=%s testName=%s", testId.c_str(), testName.c_str());
	KeyedArchive* currentTestArchive = NULL;

	KeyedArchive* dbUpdateData = FindOrInsertRunArchive(dbUpdateObject, "");

	currentTestArchive = new KeyedArchive();

	currentTestArchive->SetString("FileName", testFileName);
	currentTestArchive->SetBool("Success", false);

	KeyedArchive* stepsArchive = new KeyedArchive();
	currentTestArchive->SetArchive(AUTOTESTING_STEPS, stepsArchive);
	SafeRelease(stepsArchive);

	dbUpdateData->SetArchive(testId, currentTestArchive);
	Logger::FrameworkDebug("AutotestingSystem::InsertTestArchive new %s", testId.c_str());

	SafeRelease(currentTestArchive);
	currentTestArchive = dbUpdateData->GetArchive(testId);

	return currentTestArchive;
}

KeyedArchive *AutotestingSystem::FindOrInsertTestStepArchive(KeyedArchive *testArchive, const String &stepId)
{
    Logger::FrameworkDebug("AutotestingSystem::FindOrInsertTestStepArchive stepId=%s", stepId.c_str());
    
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
        Logger::FrameworkDebug("AutotestingSystem::FindOrInsertTestStepArchive new %s", stepId.c_str());
    }
    SafeRelease(currentTestStepArchive);
    currentTestStepArchive = testStepsArchive->GetArchive(stepId);
    SafeRelease(testStepsArchive);
    
	//Logger::FrameworkDebug("AutotestingSystem::FindOrInsertTestStepArchive finish");
    return currentTestStepArchive;
}

KeyedArchive *AutotestingSystem::FindStepArchive(KeyedArchive *testArchive, const String &stepId)
{
	Logger::FrameworkDebug("AutotestingSystem::FindStepArchive stepId=%s", stepId.c_str());

	KeyedArchive* currentTestStepArchive = NULL;
	KeyedArchive* testStepsArchive = NULL;

	testStepsArchive = SafeRetain(testArchive->GetArchive(AUTOTESTING_STEPS, NULL));
    if(testStepsArchive)
    {
        currentTestStepArchive = SafeRetain(testStepsArchive->GetArchive(stepId, NULL));
    }
	SafeRelease(testStepsArchive);

	return currentTestStepArchive;
}

KeyedArchive *AutotestingSystem::InsertStepArchive(KeyedArchive *testArchive, const String &stepId, const String &description)
{
	Logger::FrameworkDebug("AutotestingSystem::InsertStepArchive stepId=%s description=%s", stepId.c_str(), description.c_str());

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
	Logger::FrameworkDebug("AutotestingSystem::InsertStepArchive new %s", stepId.c_str());
	
	SafeRelease(currentStepArchive);
	currentStepArchive = testStepsArchive->GetArchive(stepId);
	SafeRelease(testStepsArchive);

	return currentStepArchive;
}
    
KeyedArchive *AutotestingSystem::FindOrInsertTestStepLogEntryArchive(KeyedArchive *testStepArchive, const String &logId)
{
    Logger::FrameworkDebug("AutotestingSystem::FindOrInsertTestStepLogEntryArchive logId=%s", logId.c_str());
    
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
    
	//Logger::FrameworkDebug("AutotestingSystem::FindOrInsertTestStepLogEntryArchive finish");
    return currentTestStepLogEntryArchive;
}
    
uint64 AutotestingSystem::GetCurrentTimeMS()
{
    uint64 timeAbsMs = SystemTimer::Instance()->FrameStampTimeMS();
    timeAbsMs -= startTimeMS;
    return timeAbsMs;
}

// Multiplayer API
void AutotestingSystem::WriteState(const String & device, const String & state)
{
	Logger::FrameworkDebug("AutotestingSystem::WriteState device=%s state=%s", device.c_str(), state.c_str());

	MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
	KeyedArchive* currentRunArchive = FindOrInsertRunArchive(dbUpdateObject, "_multiplayer");

	currentRunArchive->SetString(device, state);

	//SafeRelease(multiplayerArchive);
	SaveToDB(dbUpdateObject);
	SafeRelease(dbUpdateObject);

	//Logger::FrameworkDebug("AutotestingSystem::WriteState finish");
}

String AutotestingSystem::ReadState(const String & device)
{
	Logger::FrameworkDebug("AutotestingSystem::ReadState device=%s", device.c_str());
	

	MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
		KeyedArchive* currentRunArchive = FindOrInsertRunArchive(dbUpdateObject, "_multiplayer");
	String result;

	result = currentRunArchive->GetString(device.c_str(), "not_found");
	SafeRelease(dbUpdateObject);
	Logger::FrameworkDebug("AutotestingSystem::ReadState device=%s: '%s'", device.c_str(), result.c_str());
	return result;
}

void AutotestingSystem::WriteCommand(const String & device, const String & command)
{
	Logger::FrameworkDebug("AutotestingSystem::WriteCommand device=%s command=%s", device.c_str(), command.c_str());

	MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
	KeyedArchive* currentRunArchive = FindOrInsertRunArchive(dbUpdateObject, "_multiplayer");
	
	currentRunArchive->SetString(device + "_command", command);

	SaveToDB(dbUpdateObject);
	SafeRelease(dbUpdateObject);
	//Logger::FrameworkDebug("AutotestingSystem::WriteCommand finish");
}

String AutotestingSystem::ReadCommand(const String & device)
{
	Logger::FrameworkDebug("AutotestingSystem::ReadCommand device=%s", device.c_str());

	MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
	KeyedArchive* currentRunArchive = FindOrInsertRunArchive(dbUpdateObject, "_multiplayer");

	String result;
	result = currentRunArchive->GetString(device + "_command", "not_found");

	SafeRelease(dbUpdateObject);
	
	Logger::FrameworkDebug("AutotestingSystem::ReadCommand device=%s: '%s'", device.c_str(), result.c_str());
	return result;
}

void AutotestingSystem::InitializeDevice(const String & device)
{
	Logger::FrameworkDebug("AutotestingSystem::InitializeDevice device=%s", device.c_str());
	deviceName = device.c_str();
}

String AutotestingSystem::GetCurrentTimeString()
{
    uint64 timeAbsMs = GetCurrentTimeMS();

    uint16 hours = (timeAbsMs/3600000)%12;
    uint16 minutes = (timeAbsMs/60000)%60;
    uint16 seconds = (timeAbsMs/1000)%60;
	//Logger::FrameworkDebug("TIME: %02d:%02d:%02d", hours, minutes, seconds);
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
	Logger::FrameworkDebug("AutotestingSystem::OnTestStart %s", _testName.c_str());
    testName = _testName;
    
    String testId = GetTestId(testIndex);
    MongodbUpdateObject *dbUpdateObject = new MongodbUpdateObject();
    KeyedArchive *currentTestArchive = FindOrInsertTestArchive(dbUpdateObject, testId);
    currentTestArchive->SetString("Name", testName);
    SaveToDB(dbUpdateObject);
    SafeRelease(dbUpdateObject);
    
    Log("DEBUG", Format("OnTestStart %s", testName.c_str()));
}

void AutotestingSystem::OnStepStart(const String &stepName)
{
	Logger::FrameworkDebug("AutotestingSystem::OnStepStart %s", stepName.c_str());

	OnStepFinished();

	String testId = GetTestId(testIndex);
	String stepId = GetStepId(++stepIndex);

	MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
	KeyedArchive* currentTestArchive = FindOrInsertTestArchive(dbUpdateObject, testId); //FindTestArchive(dbUpdateObject, testId);	

	InsertStepArchive(currentTestArchive, stepId, stepName);

	SaveToDB(dbUpdateObject);
	SafeRelease(dbUpdateObject);
}
    
bool AutotestingSystem::CheckKeyedArchivesEqual(const String &name, KeyedArchive* firstKeyedArchive, KeyedArchive* secondKeyedArchive)
{
    bool isEqual = false;
    
    if(firstKeyedArchive && secondKeyedArchive)
    {    
        if(firstKeyedArchive == secondKeyedArchive)
        {
            isEqual = true;
        }
        else
        {
            Map<String, VariantType*> firstData = firstKeyedArchive->GetArchieveData();
            Map<String, VariantType*> secondData = secondKeyedArchive->GetArchieveData();
            if(firstData.size() == secondData.size())
            {
                isEqual = true;
                Map<String, VariantType*>::iterator firstIt = firstData.begin();
                Map<String, VariantType*>::iterator firstEndIt = firstData.end();
                
                for(; firstIt != firstEndIt; ++firstIt)
                {
                    Map<String, VariantType*>::iterator secondEndIt = secondData.end();
                    Map<String, VariantType*>::iterator secondFindIt = secondData.find(firstIt->first);
                    if(secondFindIt != secondEndIt)
                    {
                        if(secondFindIt->second != firstIt->second)
                        {
                            // pointers to VariantType are not equal, check values
                            if(secondFindIt->second && firstIt->second)
                            {
                                switch (firstIt->second->GetType())
                                {
                                    case VariantType::TYPE_KEYED_ARCHIVE:
                                        if(secondFindIt->second->GetType() == VariantType::TYPE_KEYED_ARCHIVE)
                                        {
                                            isEqual = CheckKeyedArchivesEqual(firstIt->first, firstIt->second->AsKeyedArchive(), secondFindIt->second->AsKeyedArchive());
                                        }
                                        else
                                        {
                                            isEqual = false;
                                            Logger::Error("AutotestingSystem::CheckKeyedArchivesEqual %s %x %x ERROR values for %s cannot be compared (second type is %d, expected %d TYPE_KEYED_ARCHIVE)", name.c_str(), firstKeyedArchive, secondKeyedArchive, firstIt->first.c_str(), firstIt->second->GetType(), secondFindIt->second->GetType(), firstIt->second->GetType());
                                        }
                                        break;
                                    case VariantType::TYPE_STRING:
                                        if(secondFindIt->second->GetType() == VariantType::TYPE_STRING)
                                        {
                                            if(secondFindIt->second->AsString() != firstIt->second->AsString())
                                            {
                                                
                                                isEqual = false;
                                                Logger::Error("AutotestingSystem::CheckKeyedArchivesEqual %s %x %x ERROR values for %s are not equal (%s != %s)", name.c_str(), firstKeyedArchive, secondKeyedArchive, firstIt->first.c_str(), firstIt->second->AsString().c_str(), secondFindIt->second->AsString().c_str());
                                            }
                                        }
                                        else
                                        {
                                            isEqual = false;
                                            Logger::Error("AutotestingSystem::CheckKeyedArchivesEqual %s %x %x ERROR values for %s cannot be compared (second type is %d, expected %d TYPE_STRING)", name.c_str(), firstKeyedArchive, secondKeyedArchive, firstIt->first.c_str(), firstIt->second->GetType(), secondFindIt->second->GetType(), firstIt->second->GetType());
                                        }
                                        break;
                                    case VariantType::TYPE_FLOAT:
                                        if(secondFindIt->second->GetType() == VariantType::TYPE_FLOAT)
                                        {
                                            if(firstIt->second->AsFloat() != secondFindIt->second->AsFloat())
                                            {
                                                isEqual = false;
                                                Logger::Error("AutotestingSystem::CheckKeyedArchivesEqual %s %x %x ERROR values for %s are not equal (%f != %f)", name.c_str(), firstKeyedArchive, secondKeyedArchive, firstIt->first.c_str(), firstIt->second->AsFloat(), secondFindIt->second->AsFloat());
                                            }
                                        }
                                        else
                                        {
                                            isEqual = false;
                                            Logger::Error("AutotestingSystem::CheckKeyedArchivesEqual %s %x %x ERROR values for %s cannot be compared (second type is %d, expected %d TYPE_FLOAT)", name.c_str(), firstKeyedArchive, secondKeyedArchive, firstIt->first.c_str(), firstIt->second->GetType(), secondFindIt->second->GetType(), firstIt->second->GetType());
                                        }
                                        break;
                                    case VariantType::TYPE_INT32:
                                        if(secondFindIt->second->GetType() == VariantType::TYPE_INT32)
                                        {
                                            if(firstIt->second->AsInt32() != secondFindIt->second->AsInt32())
                                            {
                                                isEqual = false;
                                                Logger::Error("AutotestingSystem::CheckKeyedArchivesEqual %s %x %x ERROR values for %s are not equal (%d != %d)", name.c_str(), firstKeyedArchive, secondKeyedArchive, firstIt->first.c_str(), firstIt->second->AsInt32(), secondFindIt->second->AsInt32());
                                            }
                                        }
                                        else
                                        {
                                            isEqual = false;
                                            Logger::Error("AutotestingSystem::CheckKeyedArchivesEqual %s %x %x ERROR values for %s cannot be compared (second type is %d, expected %d TYPE_INT32)", name.c_str(), firstKeyedArchive, secondKeyedArchive, firstIt->first.c_str(), firstIt->second->GetType(), secondFindIt->second->GetType(), firstIt->second->GetType());
                                        }
                                        break;
                                    case VariantType::TYPE_BOOLEAN:
                                        if(secondFindIt->second->GetType() == VariantType::TYPE_BOOLEAN)
                                        {
                                            if(firstIt->second->AsBool() != secondFindIt->second->AsBool())
                                            {
                                                isEqual = false;
                                                Logger::Error("AutotestingSystem::CheckKeyedArchivesEqual %s %x %x ERROR values for %s are not equal (%d != %d)", name.c_str(), firstKeyedArchive, secondKeyedArchive, firstIt->first.c_str(), firstIt->second->AsBool(), secondFindIt->second->AsBool());
                                            }
                                        }
                                        else
                                        {
                                            if(secondFindIt->second->GetType() == VariantType::TYPE_INT32)
                                            {
                                                bool secondValue = (secondFindIt->second->AsInt32() == 1);
                                                if(firstIt->second->AsBool() != secondValue)
                                                {
                                                    isEqual = false;
                                                    Logger::Error("AutotestingSystem::CheckKeyedArchivesEqual %s %x %x ERROR values for %s are not equal (%d != %d)", name.c_str(), firstKeyedArchive, secondKeyedArchive, firstIt->first.c_str(), firstIt->second->AsBool(), secondFindIt->second->AsInt32());
                                                }
                                            }
                                            else
                                            {
                                                isEqual = false;
                                                Logger::Error("AutotestingSystem::CheckKeyedArchivesEqual %s %x %x ERROR values for %s cannot be compared (second type is %d, expected %d TYPE_BOOLEAN or %d TYPE_INT32)", name.c_str(), firstKeyedArchive, secondKeyedArchive, firstIt->first.c_str(), firstIt->second->GetType(), secondFindIt->second->GetType(), firstIt->second->GetType(), VariantType::TYPE_INT32);
                                            }
                                        }
                                        break;
                                    default:
                                        //TODO: other pointer types (not only KeyedArchive) which may be equal even when pointers are not equal
                                        isEqual = false;
                                        Logger::Error("AutotestingSystem::CheckKeyedArchivesEqual %s %x %x ERROR values for %s are not equal", name.c_str(), firstKeyedArchive, secondKeyedArchive, firstIt->first.c_str());
                                        break;
                                }
                            }
                            else
                            {
                                isEqual = false;
                                Logger::Error("AutotestingSystem::CheckKeyedArchivesEqual %s %x %x ERROR one of the values for %s is NULL", name.c_str(), firstKeyedArchive, secondKeyedArchive, firstIt->first.c_str());
                            }
                        }
                    }
                    else
                    {
                        isEqual = false;
                        Logger::Error("AutotestingSystem::CheckKeyedArchivesEqual %s %x %x ERROR second archive doesn't have key %s", name.c_str(), firstKeyedArchive, secondKeyedArchive, firstIt->first.c_str());
                    }
                }
            }
            else
            {
                Logger::Error("AutotestingSystem::CheckKeyedArchivesEqual %s %x %x ERROR keys count are not equal", name.c_str(), firstKeyedArchive, secondKeyedArchive);
            }
        }
    }
    
    return isEqual;
}
    
bool AutotestingSystem::CheckSavedObjectInDB(MongodbUpdateObject *dbUpdateObject)
{
    bool isSavedObjectValid = false;
    if(dbClient)
    {
        MongodbUpdateObject *foundObject = new MongodbUpdateObject();
        isSavedObjectValid = dbClient->FindObjectByKey(dbUpdateObject->GetObjectName(), foundObject);
        if(isSavedObjectValid)
        {
            foundObject->LoadData();
            isSavedObjectValid = CheckKeyedArchivesEqual(dbUpdateObject->GetObjectName(), dbUpdateObject->GetData(), foundObject->GetData());
        }
        SafeRelease(foundObject);
    }
    return isSavedObjectValid;
}
    
bool AutotestingSystem::SaveToDB(MongodbUpdateObject *dbUpdateObject)
{
	uint64 startTime = SystemTimer::Instance()->AbsoluteMS();
	Logger::FrameworkDebug("AutotestingSystem::SaveToDB");

    bool ret = dbUpdateObject->SaveToDB(dbClient);

    if(!ret)
    {
        Logger::Error("AutotestingSystem::SaveToDB failed");
    }

	uint64 finishTime = SystemTimer::Instance()->AbsoluteMS();
	Logger::FrameworkDebug("AutotestingSystem::SaveToDB FINISH result time %d", finishTime - startTime);
	return ret;
	/*
    else
    {

		Logger::FrameworkDebug("AutotestingSystem::SaveToDB Ok");
		return ret;
		int32 maxAttemptsToWait = 1;
        int32 attemptsToWaitLeft = maxAttemptsToWait;
		int32 maxAttemptsToRetry = 5;
		int32 attemptsToRetryLeft = maxAttemptsToRetry;

		Logger::FrameworkDebug("AutotestingSystem::SaveToDB CheckSavedObjectInDB wait=%d retry=%d", attemptsToWaitLeft, attemptsToRetryLeft);

        while(!CheckSavedObjectInDB(dbUpdateObject))
        {
            if(--attemptsToWaitLeft <= 0)
            {
				if(--attemptsToRetryLeft <= 0)
				{
					Logger::Error("AutotestingSystem::SaveToDB failed, retried %d times", maxAttemptsToRetry);
				}
				else
				{
					attemptsToWaitLeft = maxAttemptsToWait;
					--attemptsToRetryLeft;

					while(!dbUpdateObject->SaveToDB(dbClient))
					{
						if(--attemptsToRetryLeft <= 0)
						{
							Logger::Error("AutotestingSystem::SaveToDB failed, retried %d times", maxAttemptsToRetry);
							break;
						}

						Logger::FrameworkDebug("AutotestingSystem::SaveToDB retry failed wait=%d retry=%d, sleep 1 sec", attemptsToWaitLeft, attemptsToRetryLeft);
#if !defined( _WIN32 )
            //sleep( 1 );
#else
            //Sleep( 1000 );
#endif
					}
				}

				if(attemptsToRetryLeft <= 0)
				{
					Logger::Error("AutotestingSystem::SaveToDB failed, retried %d times", maxAttemptsToRetry);
					break;
				}
				else
				{
					Logger::Warning("AutotestingSystem::SaveToDB failed. Attempts to retry left: %d", attemptsToRetryLeft);
				}
            }

			Logger::FrameworkDebug("AutotestingSystem::SaveToDB CheckSavedObjectInDB failed wait=%d retry=%d, sleep 1 sec", attemptsToWaitLeft, attemptsToRetryLeft);
#if !defined( _WIN32 )
            sleep( 1 );
#else
            Sleep( 1000 );
#endif
        }
    }
    return ret;*/
}

void AutotestingSystem::Log(const String &level, const String &message)
{
	Logger::FrameworkDebug("AutotestingSystem::Log [%s]%s", level.c_str(), message.c_str());
	uint64 startTime = SystemTimer::Instance()->AbsoluteMS();
	String testId = GetTestId(testIndex);
	String stepId = GetStepId(stepIndex);
	String logId = GetLogId(++logIndex);

	MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
	KeyedArchive* currentTestArchive = FindOrInsertTestArchive(dbUpdateObject, testId);//FindTestArchive(dbUpdateObject, testId);
	KeyedArchive* currentStepArchive = FindOrInsertTestStepArchive(currentTestArchive, stepId); //FindStepArchive(currentTestArchive, stepId);

	//KeyedArchive* logsArchive = currentStepArchive->GetArchive(AUTOTESTING_LOG, NULL);
	//KeyedArchive* logEntry = new KeyedArchive();
	KeyedArchive* logEntry = FindOrInsertTestStepLogEntryArchive(currentStepArchive, logId);

	logEntry->SetString("Type", level);
	String currentTime = GetCurrentTimeString();
	logEntry->SetString("Time", currentTime);
	logEntry->SetString("Message", message);

	//logsArchive->SetArchive(logId, logEntry);

    SaveToDB(dbUpdateObject);
	SafeRelease(dbUpdateObject);
	//Logger::FrameworkDebug("AutotestingSystem::Log finish");
	uint64 finishTime = SystemTimer::Instance()->AbsoluteMS();
	Logger::FrameworkDebug("AutotestingSystem::Log FINISH  summary time %d", finishTime - startTime);
}

void AutotestingSystem::SaveScreenShotNameToDB()
{
	Logger::FrameworkDebug("AutotestingSystem::SaveScreenShotNameToDB %s", screenShotName.c_str());
	
	String testId = GetTestId(testIndex);
	String stepId = GetStepId(stepIndex);

	MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
	KeyedArchive* currentTestArchive = FindOrInsertTestArchive(dbUpdateObject, testId);//FindTestArchive(dbUpdateObject, testId);
	KeyedArchive* currentStepArchive = FindOrInsertTestStepArchive(currentTestArchive, stepId); //FindStepArchive(currentTestArchive, stepId);

	currentStepArchive->SetString("screenshot", screenShotName);

	SaveToDB(dbUpdateObject);
	SafeRelease(dbUpdateObject);
}

void AutotestingSystem::OnStepFinished()
{
	Logger::FrameworkDebug("AutotestingSystem::OnStepFinished");

	// Mark step as SUCCESS
	String testId = GetTestId(testIndex);
	String stepId = GetStepId(stepIndex);
	logIndex = 0;

	MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
	KeyedArchive* currentTestArchive = FindOrInsertTestArchive(dbUpdateObject, testId);//FindTestArchive(dbUpdateObject, testId);
	if(currentTestArchive)
	{
		KeyedArchive* currentStepArchive = FindOrInsertTestStepArchive(currentTestArchive, stepId);//FindStepArchive(currentTestArchive, stepId);

		if (currentStepArchive)
		{
			currentStepArchive->SetBool("Success", true);
		}
	}
	else
	{
		OnError(Format("AutotestingSystem::OnStepFinished test %s not found", testId.c_str()));
	}

	SaveToDB(dbUpdateObject);
	SafeRelease(dbUpdateObject);
}

//deprecated
void AutotestingSystem::SaveTestStepToDB(const String &stepDescription, bool isPassed, const String &error)
{
	return;
    //if(!isDB) return;
    
    Logger::FrameworkDebug("AutotestingSystem::SaveTestStepToDB %s %d %s", stepDescription.c_str(), isPassed, error.c_str());
    
    String testId = GetTestId(testIndex);
    String stepId = GetStepId(++stepIndex);
    
    MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
    KeyedArchive* currentTestArchive = FindOrInsertTestArchive(dbUpdateObject, testId);
    KeyedArchive* currentTestStepArchive = FindOrInsertTestStepArchive(currentTestArchive, stepId);
    
    currentTestStepArchive->SetBool("Success", isPassed);
    currentTestStepArchive->SetString("Description", stepDescription);
    
    Logger::FrameworkDebug("AutotestingSystem::SaveTestStepToDB testId=%s stepId=%s", testId.c_str(), stepId.c_str());
    
    if(!error.empty())
    {
        String logId = GetLogId(++logIndex);
        KeyedArchive* currentLogEntryArchive = FindOrInsertTestStepLogEntryArchive(currentTestStepArchive, logId);
        
        currentLogEntryArchive->SetString("Type", "ERROR");
        String currentTime = GetCurrentTimeString();
        Logger::FrameworkDebug("currentTime=%s", currentTime.c_str());
        currentLogEntryArchive->SetString("Time", currentTime);
        currentLogEntryArchive->SetString("Message", error);
    }
    logIndex = 0;
    
    SaveToDB(dbUpdateObject);
    SafeRelease(dbUpdateObject);
}
    
//deprecated
void AutotestingSystem::SaveTestStepLogEntryToDB(const String &type, const String &time, const String &message)
{
	return;
    //if(!isDB) return;
    
    Logger::FrameworkDebug("AutotestingSystem::SaveTestStepLogEntryToDB %s %s %s", type.c_str(), time.c_str(), message.c_str());
    
    String testId = GetTestId(testIndex);
    String stepId = GetStepId(stepIndex);
    String logId = GetLogId(++logIndex);

    String testsName = Format("%u",testsDate);
    
    MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
    KeyedArchive* currentTestArchive = FindOrInsertTestArchive(dbUpdateObject, testId);
    KeyedArchive* currentTestStepArchive = FindOrInsertTestStepArchive(currentTestArchive, stepId);
    KeyedArchive* currentLogEntryArchive = FindOrInsertTestStepLogEntryArchive(currentTestStepArchive, logId);

    currentLogEntryArchive->SetString("Type", type);
    currentLogEntryArchive->SetString("Time", time);
    currentLogEntryArchive->SetString("Message", message);
    
    SaveToDB(dbUpdateObject);
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
#ifdef AUTOTESTING_LUA
        AutotestingSystemLua::Instance()->Update(timeElapsed);
#else
        AutotestingSystemYaml::Instance()->Update(timeElapsed);
#endif
    }
    else if(isWaiting)
    {
        waitTimeLeft -= timeElapsed;
        waitCheckTimeLeft -= timeElapsed;
        
        if(waitTimeLeft <= 0.0f)
        {
            isWaiting = false;
            isRunning = false;
            OnError("Multiplayer Wait Timeout");
        }
        else if(waitCheckTimeLeft <= 0.0f)
        {
            waitCheckTimeLeft = 0.1f;
            if(CheckMasterHelpersReadyDB())
            {
                isWaiting = false;
                isRunning = true;
            }
        }
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
    Logger::FrameworkDebug("AutotestingSystem::OnTestsStarted");
    
    startTimeMS = SystemTimer::Instance()->FrameStampTimeMS();
    
#ifdef AUTOTESTING_LUA
    AutotestingSystemLua::Instance()->StartTest();
#endif
}
    
#define AUTOTEST_MASTER_ID "master"
String AutotestingSystem::ReadMasterIDFromDB()
{
    //TODO: get first available master
    return AUTOTEST_MASTER_ID;
}
    
void AutotestingSystem::InitMultiplayer(bool _isMaster)
{
    if(!isInitMultiplayer)
    {
        isInitMultiplayer = true;
 
        isMaster = _isMaster;
        masterId = ReadMasterIDFromDB(); //TODO: DB set or get name
        
        multiplayerName = Format("%u_multiplayer", testsDate);
        Logger::FrameworkDebug("AutotestingSystem::InitMultiplayer %s", multiplayerName.c_str());
        isRunning = false;
        isWaiting = true;
        waitTimeLeft = 300.0f;
    }
}
    
void AutotestingSystem::RegisterMasterInDB(int32 helpersCount)
{
    requestedHelpers = helpersCount;
    
    MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
    if(!dbClient->FindObjectByKey(multiplayerName, dbUpdateObject))
    {
        dbUpdateObject->SetObjectName(multiplayerName);
    }
    dbUpdateObject->LoadData();
    
    KeyedArchive* masterArchive = SafeRetain(dbUpdateObject->GetData()->GetArchive(masterId, NULL));
    if(!masterArchive)
    {
        masterArchive = new KeyedArchive();
    }
    
    masterArchive->SetInt32("requested", requestedHelpers);
    masterArchive->SetInt32("helpers", 0);
    
    masterRunId = masterArchive->GetInt32("runId", 0) + 1;
    masterArchive->SetInt32("runId", masterRunId);
    
    masterArchive->SetInt32("run", 0);
    //TODO: set task for helpers into DB
    masterArchive->SetString("task", testFileName);
    
    dbUpdateObject->GetData()->SetArchive(masterId, masterArchive);
    
    isRegistered = SaveToDB(dbUpdateObject);
    Logger::FrameworkDebug("AutotestingSystem::RegisterMasterInDB %d", isRegistered);
    
    // delete created archives
    SafeRelease(masterArchive);
    
    // delete created update object
    SafeRelease(dbUpdateObject);
}
    
void AutotestingSystem::RegisterHelperInDB()
{
    MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
    if(dbClient->FindObjectByKey(multiplayerName, dbUpdateObject))
    {
        dbUpdateObject->LoadData();
        
        KeyedArchive* masterArchive = SafeRetain(dbUpdateObject->GetData()->GetArchive(masterId, NULL));
        if(masterArchive)
        {
            if(masterArchive->GetInt32("run", 0) == 0)
            {
                int32 helpersCount = masterArchive->GetInt32("helpers", 0) + 1;
                masterArchive->SetInt32("helpers", helpersCount);
                
                masterRunId = masterArchive->GetInt32("runId", 0);
                
                dbUpdateObject->GetData()->SetArchive(masterId, masterArchive);
                
                isRegistered = SaveToDB(dbUpdateObject);
                Logger::FrameworkDebug("AutotestingSystem::RegisterHelperInDB %d", isRegistered);
            }
        }
        // delete created archives
        SafeRelease(masterArchive);
    }
    // delete created update object
    SafeRelease(dbUpdateObject);
}
    
bool AutotestingSystem::CheckMasterHelpersReadyDB()
{
    //Logger::FrameworkDebug("AutotestingSystem::CheckMasterHelpersReadyDB");
    bool isReady = false;
    
    if(!isRegistered)
    {
        if(isMaster)
        {
            RegisterMasterInDB(requestedHelpers);
        }
        else
        {
            RegisterHelperInDB();
        }
    }
    
    if(isRegistered)
    {
        MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
        if(dbClient->FindObjectByKey(multiplayerName, dbUpdateObject))
        {
            dbUpdateObject->LoadData();
            KeyedArchive* masterArchive = SafeRetain(dbUpdateObject->GetData()->GetArchive(masterId, NULL));
            if(masterArchive)
            {
                // for registered agents, check if runId is overwritten
                if(masterRunId != masterArchive->GetInt32("runId"))
                {
                    // runId was overwritten
                    if(isMaster)
                    {
                        // exit on error (conflict with another master)
                        OnError("Multiplayer Master Conflict");
                    }
                    else
                    {
                        // re-register helper
                        isRegistered = false;
                    }
                }
                else
                {
                    if(isMaster)
                    {
                        int32 requested = masterArchive->GetInt32("requested");
                        int32 helpers = masterArchive->GetInt32("helpers");
                        if(requested == helpers)
                        {
                            if(requested != requestedHelpers)
                            {
                                OnError("Multiplayer Master wrong Helpers count");
                            }
                            else
                            {
                                masterArchive->SetInt32("run", 1);
                                
                                dbUpdateObject->GetData()->SetArchive(masterId, masterArchive);
                                
                                isReady = SaveToDB(dbUpdateObject);
                                if(isReady)
                                {
                                    Logger::FrameworkDebug("AutotestingSystem::CheckMasterHelpersReadyDB Master: %d helpers ready", requestedHelpers);
                                }
                                else
                                {
                                    Logger::FrameworkDebug("AutotestingSystem::CheckMasterHelpersReadyDB Master: failed to run");
                                }
                            }
                        }
                    }
                    else
                    {
                        
                        if(masterArchive->GetInt32("run", 0) > 0)
                        {
                            isReady = true;
                            //TODO: get task from DB
                            masterTask = masterArchive->GetString("task");
                            Logger::FrameworkDebug("AutotestingSystem::CheckMasterHelpersReadyDB Helper: run test %s", masterTask.c_str());
                        }
                    }
                }
            }
            SafeRelease(masterArchive);
        }
        SafeRelease(dbUpdateObject);
    }
    return isReady;
}

void AutotestingSystem::OnTestStep(const String & stepName, bool isPassed, const String &error)
{
    String assertMsg = Format("%s: %s %s", testName.c_str(), stepName.c_str(), (isPassed ? "PASSED" : "FAILED"));
    Logger::FrameworkDebug("AutotestingSystem::OnTestStep %s", assertMsg.c_str());
    
    //AddTestResult(stepName, isPassed, error);
	//SaveTestToDB();
    
    SaveTestStepToDB(stepName, isPassed, error);
    
	if(reportFile)
	{
		reportFile->WriteLine(assertMsg);
	}

	if(!isPassed)
	{
		SafeRelease(reportFile);
		ExitApp();
	}
}

void AutotestingSystem::OnMessage(const String & logMessage)
{
	Logger::FrameworkDebug("AutotestingSystem::OnMessage %s", logMessage.c_str());
    
	String logMsg = Format("%s OnMessage %s", testName.c_str(), logMessage.c_str());
	if(reportFile)
	{
		reportFile->WriteLine(logMsg);
	}
}

void AutotestingSystem::OnError(const String & errorMessage)
{
    Logger::Error("AutotestingSystem::OnError %s",errorMessage.c_str());
	
	Log("ERROR", errorMessage);
    //SaveTestStepLogEntryToDB("ERROR", GetCurrentTimeString(), errorMessage);
	
	MakeScreenShot();
	SaveScreenShotNameToDB();
    
	if (deviceName != "not-initialized")
	{
		WriteState(deviceName, "error");
	}

	String exitOnErrorMsg = Format("EXIT %s OnError %s", testName.c_str(), errorMessage.c_str());
	if(reportFile)
	{
		reportFile->WriteLine(exitOnErrorMsg);
	}
	SafeRelease(reportFile);

    ExitApp();
}

void AutotestingSystem::MakeScreenShot()
{
	Logger::FrameworkDebug("AutotestingSystem::MakeScreenShot");
	uint64 timeAbsMs = GetCurrentTimeMS();
    uint16 hours = (timeAbsMs/3600000)%60;
    uint16 minutes = (timeAbsMs/60000)%60;
    uint16 seconds = (timeAbsMs/1000)%60;
	screenShotName = Format("%s_%s_%02d_%02d_%02d", AUTOTESTING_PLATFORM_NAME, groupName.c_str(), hours, minutes, seconds);

	RenderManager::Instance()->RequestGLScreenShot(this);
}

String AutotestingSystem::GetScreenShotName()
{
	Logger::FrameworkDebug("AutotestingSystem::GetScreenShotName %s", screenShotName.c_str());
	return screenShotName.c_str();
}

void AutotestingSystem::OnScreenShot(Image *image)
{
	Logger::FrameworkDebug("AutotestingSystem::OnScreenShot %s", screenShotName.c_str());
	uint64 startTime = SystemTimer::Instance()->AbsoluteMS();
	FilePath systemFilePath = "~doc:/screenshot.png";
	image->ResizeImage(1024, 768);
	image->ResizeCanvas(1024, 768);

	if(dbClient)
	{
		//Logger::FrameworkDebug("Image: datasize %d, %d x %d", image->dataSize, image->GetHeight(), image->GetWidth());
		dbClient->SaveBufferToGridFS(screenShotName, reinterpret_cast<char*>( image->GetData()), image->dataSize);
		
		uint64 finishTime = SystemTimer::Instance()->AbsoluteMS();
		Logger::FrameworkDebug("AutotestingSystem::OnScreenShot Upload: %d", finishTime - startTime);
	}
}
    
void AutotestingSystem::OnTestsFinished()
{
    Logger::FrameworkDebug("AutotestingSystem::OnTestsFinished");
    
	// Mark last step as SUCCESS
	OnStepFinished();

	if (deviceName != "not-initialized")
	{
		WriteState(deviceName, "finished");
	}

	// Mark test as SUCCESS
	String testId = GetTestId(testIndex);
	MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
	KeyedArchive* currentTestArchive = FindOrInsertTestArchive(dbUpdateObject, testId);//FindTestArchive(dbUpdateObject, testId);

	if (currentTestArchive)
	{
		currentTestArchive->SetBool("Success", true);
	}

	SaveToDB(dbUpdateObject);
	SafeRelease(dbUpdateObject);
    
    if(reportFile)
    {
        reportFile->WriteLine(Format("EXIT %s OnTestsFinished", testName.c_str()));
    }
    SafeRelease(reportFile);

    ExitApp();
}

void AutotestingSystem::OnInput(const UIEvent &input)
{
    Logger::FrameworkDebug("AutotestingSystem::OnInput %d phase=%d count=%d point=(%f, %f) physPoint=(%f,%f) key=%c",input.tid, input.phase, input.tapCount, input.point.x, input.point.y, input.physPoint.x, input.physPoint.y, input.keyChar);
    
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