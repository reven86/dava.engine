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


#include "GameCore.h"
#include "AppScreens.h"
#include "FileManagerWrapper.h"
#include "SettingsManager.h"
#include "LandscapeTestData.h"
#include "Config.h"
#include "Database/MongodbObject.h"
#include "Platform/DeviceInfo.h"


using namespace DAVA;

GameCore::GameCore()
{
	resultScreen = NULL;
	currentRunId = -1;
	dbClient = NULL;

	cursor = 0;
}

GameCore::~GameCore()
{
	
}

void GameCore::OnAppStarted()
{
	File * testIdFile = File::Create("~res:/testId", File::OPEN | File::READ);
	if(testIdFile)
	{
		char buf[30];
		memset(buf, 0, sizeof(char)*30);
		testIdFile->ReadLine(buf, 30);
		sscanf(buf, "%d",&currentRunId);
		SafeRelease(testIdFile);
	}
	else
	{
		Logger::Debug("[GameCore::OnAppStarted()] testId file not found!");
        logToDb.push_back("'testId' file not found!");
		Core::Instance()->Quit();
		return;
	}
    
	SettingsManager::Instance()->InitWithFile("~res:/Config/config.yaml");
	
	RenderManager::Instance()->SetFPS(60);

	FilePath dirPath("~res:/3d/Maps/");
	Vector<String> levelsPaths;
	YamlParser* parser = YamlParser::Create("~res:/maps.yaml");
	if(parser)
	{
		YamlNode* rootNode = parser->GetRootNode();
		if(rootNode)
		{
			int32 sz = rootNode->GetCount();

			for(DAVA::int32 i = 0; i < sz; ++i)
			{
				String k = rootNode->GetItemKeyName(i);
				String levelFile = rootNode->Get(i)->AsString();
                if(levelFile.rfind(".sc2") != String::npos)
                {
                    File * file = File::Create(dirPath + levelFile, File::OPEN | File::READ);
                    if(file)
                    {
                        levelsPaths.push_back(levelFile);
                        Logger::Debug("[GameCore::OnAppStarted()] Add test level: %s", levelFile.c_str());
                    }
                    else
                    {
                        Logger::Debug("[GameCore::OnAppStarted()] Scenefile not found: %s", levelFile.c_str());
                        logToDb.push_back(Format("Scenefile not found: %s", levelFile.c_str()));
                    }
                    SafeRelease(file);
                }
			}
		}
	}
    else
    {
        logToDb.push_back("File '~res:/maps.yaml' not found");
    }

	for(Vector<String>::const_iterator it = levelsPaths.begin(); it != levelsPaths.end(); ++it)
    {
		Test *test = new Test(dirPath + (*it));
		if(test != NULL)
        {
			UIScreenManager::Instance()->RegisterScreen(test->GetScreenId(), test);
			tests.push_back(test);
		}
	}

    Logger::Debug("[GameCore::OnAppStarted()] test count %d", tests.size());

	if(levelsPaths.size() > 0)
    {
		appFinished = false;
		
		testCount = tests.size();
		Test *firstTest = tests.front();
		UIScreenManager::Instance()->SetFirst(firstTest->GetScreenId());
	}
    else
    {
		appFinished = true;
	}

	ConnectToDB();
}

void GameCore::OnAppFinished()
{
	if(dbClient)
	{
		dbClient->Disconnect();
		SafeRelease(dbClient);
	}

	SafeRelease(cursor);
}

void GameCore::OnSuspend()
{
    ApplicationCore::OnSuspend();
}

void GameCore::OnResume()
{
    ApplicationCore::OnResume();
}

void GameCore::OnBackground()
{
	
}

void GameCore::BeginFrame()
{
	ApplicationCore::BeginFrame();
	RenderManager::Instance()->ClearWithColor(0, 0, 0, 0);
}

void GameCore::Update(float32 timeElapsed)
{
	ApplicationCore::Update(timeElapsed);
	
	if(!appFinished)
    {
		Test *curTest = tests.front();
        if(curTest->IsFinished())
        {
			if(resultScreen == NULL)
            {
				resultScreen = new ResultScreen(curTest->GetLandscapeTestData(),
												curTest->GetFilePath(),
												curTest->GetLandscapeTexture());
                
				UIScreenManager::Instance()->RegisterScreen(RESULT_SCREEN, resultScreen);
                UIScreenManager::Instance()->SetScreen(RESULT_SCREEN);
			}
			
			if(resultScreen->IsFinished())
            {
				tests.pop_front();

                Logger::Debug("[GameCore::Update()] test count %d", tests.size());

				if(tests.size() == 0)
                {
                    appFinished = true;
                    Logger::Debug("[GameCore::Update()] All tests finished");
				}
                else
                {
                    SafeRelease(resultScreen);
                    Test *newCurTest = tests.front();
                    Logger::Debug("[GameCore::Update()] Start next test");
					if(newCurTest != NULL)
                    {
						UIScreenManager::Instance()->SetScreen(newCurTest->GetScreenId());
					}
				}

				SafeRelease(curTest);
			}
		}
	}
    else
    {
        if(!FlushLogToDB())
            Logger::Debug("Error sending data to DB (connection is lost) !!!");

		Core::Instance()->Quit();
	}
}

void GameCore::Draw()
{
	ApplicationCore::Draw();
}

bool GameCore::ConnectToDB()
{
	if(dbClient && dbClient->IsConnected())
		return true;

    if(dbClient)
        SafeRelease(dbClient);
    
	dbClient = MongodbClient::Create(DATABASE_IP, DATAPASE_PORT);
    
	if(dbClient)
    {
        dbClient->SetDatabaseName(DATABASE_NAME);
        dbClient->SetCollectionName(DATABASE_COLLECTION);

		//MongodbObject *globalIdObject = dbClient->FindObjectByKey("GlobalTestId");

		//if(globalIdObject)
		//	currentRunId = globalIdObject->GetInt32("LastTestId") + 1;

		//MongodbObject * newGlobalIdObject = new MongodbObject();
		//newGlobalIdObject->SetObjectName("GlobalTestId");
		//newGlobalIdObject->AddInt32("LastTestId", currentRunId);
		//newGlobalIdObject->Finish();
		//dbClient->SaveObject(newGlobalIdObject, globalIdObject);

		//SafeRelease(newGlobalIdObject);
		//SafeRelease(globalIdObject);
    }
    else
    {
        Logger::Debug("Can't connect to DB");
    }
    
    return (dbClient != NULL);
}

bool GameCore::FlushLogToDB()
{
    ConnectToDB();

    if(!dbClient && !dbClient->IsConnected())
        return false;

    if(logToDb.size() == 0)
    {
        Logger::Debug("Error log is empty");
        return true;
    }

    Logger::Debug("Sending Log to DB...");

    MongodbObject *logObject = new MongodbObject();
    if(logObject)
    {
        logObject->SetObjectName("ErrorLog");

        for(int32 i = 0; i < logToDb.size(); ++i)
        {
            logObject->AddString(Format("%d", i), logToDb[i]);
        }

        logObject->Finish();

        logToDb.clear();

        MongodbObject * currentRunObject = dbClient->FindObjectByKey(Format("%d", currentRunId));
        MongodbObject * newRunObject = new MongodbObject();
        if(newRunObject)
        {
            if(currentRunObject)
            {
                newRunObject->Copy(currentRunObject);
            }
            else
            {
                newRunObject->SetObjectName(Format("%d", currentRunId));
            }

            newRunObject->AddObject("ErrorLog", logObject);
            newRunObject->Finish();
            dbClient->SaveObject(newRunObject, currentRunObject);
            SafeRelease(newRunObject);
        }
        else
        {
            Logger::Debug("Can't create log object in DB");
            return false;
        }

        SafeRelease(currentRunObject);
        SafeRelease(logObject);
    }
    else
    {
        Logger::Debug("Can't create tests results object");
        return false;
    }

    Logger::Debug("Log successful sent to DB");
}

bool GameCore::FlushToDB(const FilePath & levelName, const Map<String, String> &results, const FilePath &imagePath)
{
    ConnectToDB();

	if(!dbClient && !dbClient->IsConnected())
		return false;

    Logger::Debug("Sending results to DB...");
    
    MongodbObject *testResultObject = new MongodbObject();
    if(testResultObject)
    {
        testResultObject->SetObjectName(levelName.GetFilename());
        
        Map<String, String>::const_iterator it = results.begin();
        for(; it != results.end(); it++)
        {
            testResultObject->AddString((*it).first, (*it).second);
        }
        
        File * imageFile = File::Create(imagePath, File::READ | File::OPEN);
        if(imageFile)
        {
            uint32 fileSize = imageFile->GetSize();
            uint8 * data = new uint8[fileSize];
            imageFile->Read(data, fileSize);
            testResultObject->AddData("ResultImagePNG", data, fileSize);
            
            SafeDelete(data);
            SafeRelease(imageFile);
        }
        else
        {
            Logger::Debug("Can't read result level sprite!");
        }
        
        testResultObject->Finish();
        
		MongodbObject * currentRunObject = dbClient->FindObjectByKey(Format("%d", currentRunId));
		MongodbObject * newRunObject = new MongodbObject();
		if(newRunObject)
		{
			if(currentRunObject)
			{
				newRunObject->Copy(currentRunObject);
			}
			else
			{
				newRunObject->SetObjectName(Format("%d", currentRunId));
				newRunObject->AddString("DeviceDescription", DeviceInfo::GetModel() + " " + DeviceInfo::GetVersion());
			}

			newRunObject->AddObject(levelName.GetFilename(), testResultObject);
			newRunObject->Finish();
			dbClient->SaveObject(newRunObject, currentRunObject);
			SafeRelease(newRunObject);
		}
		else
		{
			Logger::Debug("Can't create test result object in DB");
			return false;
		}

		SafeRelease(currentRunObject);
		SafeRelease(testResultObject);
    }
    else
    {
        Logger::Debug("Can't create tests results object");
        return false;
    }
    
    Logger::Debug("Results successful sent to DB");
    
    return true;
}