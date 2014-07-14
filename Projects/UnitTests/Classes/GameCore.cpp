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

#include "Database/MongodbObject.h"
#include "Platform/DateTime.h"

#include "Config.h"
#include "BaseScreen.h"
#include "SampleTest.h"
#include "EntityTest.h"
#include "MemoryAllocatorsTest.h"
#include "HashMapTest.h"
#include "SoundTest.h"
#include "AlignTest.h"
#include "SplitTest.h"
#include "MaterialCompilerTest.h"
#include "PVRTest.h"
#include "DXTTest.h"
#include "KeyedArchiveYamlTest.h"
#include "CloneTest.h"
#include "DLCSystemTests.h"
#include "DPITest.h"
#include "EMailTest.h"
#include "InputTest.h"
#include "FilePathTest.h"
#include "FileListTest.h"
#include "FileSystemTest.h"
#include "DeviceInfoTest.h"
#include "LocalizationTest.h"
#include "UIListTest.h"
#include "TransparentWebViewTest.h"
#include "FormatsTest.h"
#include "UIScrollViewTest.h"
#include "ThreadSyncTest.h"
#include "UIMovieTest.h"
#include "ComponentsTest.h"
#include "RectSpriteTest.h"
#include "OpenGLES30FormatTest.h"
#include "StringFormatTest.h"
#include "SaveImageTest.h"
#include "JPEGTest.h"
#include "DateTimeTest.h"
#include "UIParticlesTest.h"
#include "SceneSystemTest.h"
#include "ParseTextTest.h"
#include "ImageSizeTest.h"

using namespace DAVA;

GameCore::GameCore()
{
    logFile = NULL;
    
	dbClient = NULL;

    currentScreen = NULL;
    
    currentScreenIndex = 0;
    currentTestIndex = 0;
}

GameCore::~GameCore()
{
}

void GameCore::OnAppStarted()
{
	RenderManager::Instance()->SetFPS(60);

    CreateDocumentsFolder();

	new ImageSizeTest();

    new PVRTest();
 	new DXTTest();
    new JPEGTest();

    new ParseTextTest();
    new OpenGLES30FormatTest();
    new SaveImageTest();
    
    new OpenGLES30FormatTest();
    new StringFormatTest();
    new RectSpriteTest();

	new ComponentsTest();
    new FilePathTest();
    new FileListTest();
    new FileSystemTest();
    
 	new UIMovieTest();
 	new InputTest();
    new FormatsTest();
 	new DeviceInfoTest();
 	new DateTimeTest();
 	new TransparentWebViewTest();
    new LocalizationTest();
 
 	new SampleTest();
 	new EntityTest(); 
 	new MemoryAllocatorsTest();
 	new HashMapTest();
 	new SoundTest();
 	new SplitTest();
 	new AlignTest();
 	new EMailTest();
 	new DPITest();
 	new MaterialCompilerTest();
 	new CloneTest();

 	new EntityTest();	
 	new MemoryAllocatorsTest();
 	new HashMapTest();
 	new KeyedArchiveYamlTest();
 	new DLCTest();
 	new UIListTest();
 	new UIScrollViewTest();
 	new UIParticlesTest();
    new ThreadSyncTest();
    new SceneSystemTest();
    
    errors.reserve(TestCount());

    RunTests();
}

void GameCore::RegisterScreen(BaseScreen *screen)
{
    UIScreenManager::Instance()->RegisterScreen(screen->GetScreenId(), screen);
    screens.push_back(screen);
}


void GameCore::CreateDocumentsFolder()
{
    FilePath documentsPath = FileSystem::Instance()->GetUserDocumentsPath() + "UnitTests/";
    
    FileSystem::Instance()->CreateDirectory(documentsPath, true);
    FileSystem::Instance()->SetCurrentDocumentsDirectory(documentsPath);
}


File * GameCore::CreateDocumentsFile(const String &filePathname)
{
    FilePath workingFilepathname = FilePath::FilepathInDocuments(filePathname);
    
    
    FileSystem::Instance()->CreateDirectory(workingFilepathname.GetDirectory(), true);
    
	File *retFile = File::Create(workingFilepathname, File::CREATE | File::WRITE);
    return retFile;
}

void GameCore::OnAppFinished()
{
	int32 errorsSize = errors.size();
    for(int32 i = 0; i < errorsSize; ++i)
    {
        SafeDelete(errors[i]);
    }
    errors.clear();

    
	int32 screensSize = screens.size();
    for(int32 i = 0; i < screensSize; ++i)
    {
        SafeRelease(screens[i]);
    }
    screens.clear();

    SafeRelease(logFile);
}

void GameCore::OnSuspend()
{
//    Logger::Debug("GameCore::OnSuspend");
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    ApplicationCore::OnSuspend();
#endif

}

void GameCore::OnResume()
{
    Logger::Debug("GameCore::OnResume");
    ApplicationCore::OnResume();
}


#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)
void GameCore::OnDeviceLocked()
{
//    Logger::Debug("GameCore::OnDeviceLocked");
    Core::Instance()->Quit();
}

void GameCore::OnBackground()
{
    Logger::Debug("GameCore::OnBackground");
}

void GameCore::OnForeground()
{
	Logger::Debug("GameCore::OnForeground");
	ApplicationCore::OnForeground();
}

#endif //#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)


void GameCore::BeginFrame()
{
	ApplicationCore::BeginFrame();
	RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.f);
}

void GameCore::Update(float32 timeElapsed)
{	
    ProcessTests();
	ApplicationCore::Update(timeElapsed);
}

void GameCore::Draw()
{
	ApplicationCore::Draw();
}

void GameCore::RunTests()
{
    currentTestIndex = 0;
	int32 screensSize = screens.size();
    for(int32 iScr = 0; iScr < screensSize; ++iScr)
    {
        int32 count = screens[iScr]->GetTestCount();
        if(0 < count)
        {
            currentScreen = screens[iScr];
            currentScreenIndex = iScr;
            break;
        }
    }
    
    if(currentScreen)
    {
        UIScreenManager::Instance()->SetFirst(currentScreen->GetScreenId());
    }
    else 
    {
        LogMessage(String("There are no tests."));
        Core::Instance()->Quit();
    }
}


void GameCore::FinishTests()
{
    FlushTestResults();
    Core::Instance()->Quit();
}

void GameCore::LogMessage(const String &message)
{
    if(!logFile)
    {
        time_t logStartTime = time(0);
        logFile = CreateDocumentsFile(Format("Reports/%lld.errorlog", logStartTime));
        DVASSERT(logFile);
    }
    
    if(logFile)
    {
        logFile->WriteLine(message);
    }
}

int32 GameCore::TestCount()
{
    int32 count = 0;
    int32 screensSize = screens.size();
    for(int32 i = 0; i < screensSize; ++i)
    {
        count += screens[i]->GetTestCount();
    }
    
    return count;
}

void GameCore::ProcessTests()
{
    if(currentScreen && currentScreen->ReadyForTests())
    {
        bool ret = currentScreen->RunTest(currentTestIndex);
        if(ret)
        {
            ++currentTestIndex;
            if(currentScreen->GetTestCount() == currentTestIndex)
            {
                ++currentScreenIndex;
                if(currentScreenIndex == screens.size())
                {
                    FinishTests();
                }
                else 
                {
                    currentScreen = screens[currentScreenIndex];
                    currentTestIndex = 0;
                    UIScreenManager::Instance()->SetScreen(currentScreen->GetScreenId());
                }
            }
        }
    }
}


void GameCore::FlushTestResults()
{
    bool connected = ConnectToDB();
    if(!connected)
    {
        LogMessage(String("Can't connect to DB"));
        return;
    }

//    //TODO: test
//    dbClient->DropCollection();
//    dbClient->DropDatabase();
//    //end of test
    
    time_t logStartTime = time(0);
    String testTimeString = Format("%lld", logStartTime);

    tm* utcTime = localtime(&logStartTime);
    String runTime = Format("%04d.%02d.%02d:%02d:%02d:%02d",   
                            utcTime->tm_year + 1900, utcTime->tm_mon + 1, utcTime->tm_mday, 
                            utcTime->tm_hour, utcTime->tm_min, utcTime->tm_sec);

    
    MongodbObject *logObject = CreateLogObject(testTimeString, runTime);
    if(logObject)
    {
        MongodbObject *oldPlatformObject = dbClient->FindObjectByKey(PLATFORM_NAME);
        MongodbObject *newPlatformObject = new MongodbObject();
        if(newPlatformObject)
        {
            if(oldPlatformObject)
            {
//                oldPlatformObject->Print();
                
                newPlatformObject->Copy(oldPlatformObject);
            }
            else 
            {
                newPlatformObject->SetObjectName(PLATFORM_NAME);
            }
            
            newPlatformObject->AddObject(testTimeString, logObject);
            newPlatformObject->Finish();
            dbClient->SaveObject(newPlatformObject, oldPlatformObject);
            SafeRelease(newPlatformObject);
        }
        
        SafeRelease(oldPlatformObject);
        SafeRelease(logObject);
    }

    dbClient->Disconnect();
    SafeRelease(dbClient);
}


void GameCore::RegisterError(const String &command, const String &fileName, int32 line, TestData *testData)
{
    ErrorData *newError = new ErrorData();
    
    newError->command = command;
    newError->filename = fileName;
    newError->line = line;
    
    if(testData)
    {
        newError->testName = testData->name;
        newError->testMessage = testData->message;
    }
    else
    {
        newError->testName = String("");
        newError->testMessage = String("");
    }
    
    errors.push_back(newError);
    Logger::Error(GetErrorText(newError).c_str());
}

bool GameCore::ConnectToDB()
{
    DVASSERT(NULL == dbClient);
    
    dbClient = MongodbClient::Create(DATABASE_IP, DATAPASE_PORT);
    if(dbClient)
    {
        dbClient->SetDatabaseName(DATABASE_NAME);
        dbClient->SetCollectionName(DATABASE_COLLECTION);
    }
    
    return (NULL != dbClient);
}


MongodbObject * GameCore::CreateLogObject(const String &logName, const String &runTime)
{
    MongodbObject *logObject = new MongodbObject();
    if(logObject)
    {
        logObject->SetObjectName(logName);
    }
    
    int32 errorCount = (int32)errors.size();
    File *reportFile = CreateDocumentsFile(String("Errors.txt"));
    if(reportFile)
    {
        reportFile->WriteLine(String("Run Time: ") + runTime);
        if(logObject)
        {
            logObject->AddString(String("RunTime"), runTime);
        }

        
        if(0 < errorCount)
        {
            reportFile->WriteLine(String("Failed tests:"));
            for(int32 i = 0; i < errorCount; ++i)
            {
                String errorString = GetErrorText(errors[i]);
                
                reportFile->WriteLine(String(Format("Error[%06d]: ", i+1)) + errorString);
                if(logObject)
                {
                    logObject->AddString(String(Format("Error_%06d", i+1)), errorString);
                }
            }
        }
        else 
        {
            String successString = String("All test passed.");
            reportFile->WriteLine(successString);
            if(logObject)
            {
                logObject->AddString(String("TestResult"), successString);
            }
        }
        
        SafeRelease(reportFile);
    }

    if(logObject)
    {
        logObject->Finish();
    }
    
    return logObject;
}

const String GameCore::GetErrorText(const ErrorData *error)
{
    String errorString = String(Format("command: %s at file: %s at line: %d",
                                       error->command.c_str(), error->filename.c_str(), error->line));
    
    if(!error->testName.empty())
    {
        errorString += String(Format(", test: %s", error->testName.c_str()));
    }
    
    if(!error->testMessage.empty())
    {
        errorString += String(Format(", message: %s", error->testMessage.c_str()));
    }

    return errorString;
}


MongodbObject * GameCore::CreateSubObject(const String &objectName, MongodbObject *dbObject, bool needFinished)
{
    MongodbObject *subObject = new MongodbObject();
    if(dbObject)
    {
        bool ret = dbObject->GetSubObject(subObject, objectName, needFinished);
        if(ret)
        {
            return subObject;
        }
    }
    
    subObject->SetObjectName(objectName);
    return subObject;
}


