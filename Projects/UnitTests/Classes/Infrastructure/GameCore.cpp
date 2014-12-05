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

#include "Platform/DateTime.h"
#include "TexturePacker/CommandLineParser.h"
#include "Utils/Utils.h"

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
#include "DPITest.h"
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
#include "DFFontTest.h"
#include "ComponentsTest.h"
#include "RectSpriteTest.h"
#include "OpenGLES30FormatTest.h"
#include "StringFormatTest.h"
#include "SaveImageTest.h"
#include "JPEGTest.h"
#include "DateTimeTest.h"
#include "SceneSystemTest.h"
#include "ParseTextTest.h"
#include "ImageSizeTest.h"
#include "DLCDownloadTest.h"
#include "FunctionBindSingalTest.h"
#include "MathTest.h"
#include "BiDiTest.h"
#include "TextSizeTest.h"

#include <fstream>
#include <algorithm>

using namespace DAVA;


void GameCore::RunOnlyThisTest()
{
    //runOnlyThisTest = "TestClassName";
    //runOnlyThisTest = "TextSizeTest";
}

void GameCore::OnError()
{
    DebugBreak();
}

GameCore::GameCore():currentScreen(NULL),
    currentScreenIndex(0),
    currentTestIndex(0)
{
}

GameCore::~GameCore()
{
}

void GameCore::OnAppStarted()
{
    InitLogging();
    RunOnlyThisTest();

    RenderManager::Instance()->SetFPS(60);

    //new DLCDownloadTest();
    new MathTest();
    new FunctionBindSignalTest();
#ifndef __DAVAENGINE_ANDROID__
    new ThreadSyncTest(); // TODO this test hang on on teamcity build machine
                          // TODO this test crush on android
#endif

    new ImageSizeTest();
    //new DeviceInfoTest();
    new PVRTest();
    new DXTTest();
    new JPEGTest();

    new ParseTextTest(Font::TYPE_FT);
    //    new ParseTextTest(Font::TYPE_GRAPHICAL);
    //new OpenGLES30FormatTest();
    new SaveImageTest();
    //   
    //   new OpenGLES30FormatTest(); // TODO duplicate? second run?
    new StringFormatTest();
    //new RectSpriteTest();

    new ComponentsTest();
    //new FilePathTest();
    new FileListTest();
    new FileSystemTest();
    
    new UIMovieTest();
    //new InputTest();
    //new FormatsTest();
 
    new DateTimeTest();
    //new TransparentWebViewTest();
    new LocalizationTest();
 
    new SampleTest();
    //new EntityTest(); 
    new MemoryAllocatorsTest();
    new HashMapTest();
    //new SoundTest();
    new SplitTest();
#ifndef __DAVAENGINE_ANDROID__
    new AlignTest(); // TODO crush on android
#endif
    //new EMailTest();
    //new DPITest();
    new MaterialCompilerTest(); // TODO empty
    new CloneTest(); // TODO empty
    new BiDiTest();
#ifndef __DAVAENGINE_ANDROID__
    new TextSizeTest(); // TODO crush on android
#endif

    new EntityTest(); // TODO empty
    new KeyedArchiveYamlTest();
    //new UIListTest();
    //new UIScrollViewTest();
 

    //new SceneSystemTest();

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
    DAVA::Logger::Instance()->RemoveCustomOutput(&teamCityOutput);

    int32 screensSize = screens.size();
    for(int32 i = 0; i < screensSize; ++i)
    {
        SafeRelease(screens[i]);
    }
    screens.clear();
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
    //Core::Instance()->Quit();
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
        BaseScreen& screen = *screens[iScr];
        if (IsNeedSkipTest(screen))
        {
            continue;
        }
        int32 count = screen.GetTestCount();
        if(0 < count)
        {
            currentScreen = screens[iScr];
            currentScreenIndex = iScr;
            break;
        }
    }
    
    if(currentScreen)
    {
        Logger::Info(TeamcityTestsOutput::FormatTestStarted(currentScreen->GetTestName()).c_str());

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
    // inform teamcity script we just finished all tests
    // useful on ios devices (run with lldb)
    teamCityOutput.Output(DAVA::Logger::LEVEL_DEBUG, "Finish all tests.");
    Core::Instance()->Quit();
}

void GameCore::LogMessage(const String &message)
{
    DAVA::Logger::Error(message.c_str());
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
                    Logger::Info(TeamcityTestsOutput::FormatTestFinished(currentScreen->GetTestName()).c_str());
                    FinishTests();
                }
                else 
                {
                    Logger::Info(TeamcityTestsOutput::FormatTestFinished(currentScreen->GetTestName()).c_str());

                    currentScreen = screens[currentScreenIndex];

                    while (IsNeedSkipTest(*currentScreen))
                    {
                        ++currentScreenIndex;
                        if (currentScreenIndex == screens.size())
                        {
                            FinishTests();
                            return;
                        }
                        currentScreen = screens[currentScreenIndex];
                    }

                    currentTestIndex = 0;

                    Logger::Info(TeamcityTestsOutput::FormatTestStarted(currentScreen->GetTestName()).c_str());

                    UIScreenManager::Instance()->SetScreen(currentScreen->GetScreenId());
                }
            }
        }
    }
}

void GameCore::RegisterError(const String &command, const String &fileName, int32 line, TestData *testData)
{
    OnError();

    const char* testName = currentScreen->GetTestName().c_str();

    String errorString = String(Format("%s(%d): ",
        fileName.c_str(), line));

    if (testData)
    {
        if(!testData->name.empty())
        {
            errorString += String(Format(" %s", testData->name.c_str())); // test function name
        }

        if(!testData->message.empty())
        {
            errorString += String(Format(" %s", testData->message.c_str()));
        }
    }
    LogMessage(TeamcityTestsOutput::FormatTestFailed(testName, command, errorString));
}

DAVA::String GameCore::CreateOutputLogFile()
{
    time_t logStartTime = time(0);
    const String logFileName = Format("Reports/%lld.errorlog", logStartTime);
    File* logFile = CreateDocumentsFile(logFileName);
    DVASSERT(logFile);
    SafeRelease(logFile);

    FilePath workingFilepathname = FilePath::FilepathInDocuments(logFileName);
    return workingFilepathname.GetAbsolutePathname();
}

void GameCore::InitLogging()
{
    if (CommandLineParser::Instance()->CommandIsFound("-only_test"))
    {
        runOnlyThisTest = CommandLineParser::Instance()->GetCommandParam("-only_test");
    }

    Logger::Instance()->AddCustomOutput(&teamCityOutput);
}

bool GameCore::IsNeedSkipTest(const BaseScreen& screen) const
{
    if (runOnlyThisTest.empty())
    {
        return false;
    }

    const String& name = screen.GetTestName();

    return 0 != CompareCaseInsensitive(runOnlyThisTest, name);
}




