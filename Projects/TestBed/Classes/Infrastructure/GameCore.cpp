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


#include "Infrastructure/GameCore.h"

#include "Platform/DateTime.h"
#include "CommandLine/CommandLineParser.h"
#include "Utils/Utils.h"
#include "Infrastructure/TestListScreen.h"
#include "Tests/NotificationTest.h"
#include "Tests/UIScrollViewTest.h"
#include "Tests/SpeedLoadImagesTest.h"
#include "Tests/MultilineTest.h"
#include "Tests/StaticTextTest.h"
#include "Tests/StaticWebViewTest.h"
#include "Tests/UIMovieTest.h"
#include "Tests/FontTest.h"
#include "Tests/WebViewTest.h"
#include "Tests/FunctionSignalTest.h"
#include "Tests/KeyboardTest.h"
#include "Tests/FullscreenTest.h"
//$UNITTEST_INCLUDE

#include "MemoryManager/MemoryProfiler.h"

void GameCore::RunOnlyThisTest()
{
    //runOnlyThisTest = "NotificationScreen";
}

void GameCore::OnError()
{
    DavaDebugBreak();
}

void GameCore::RegisterTests()
{
    new UIScrollViewTest();
    new NotificationScreen();
    new SpeedLoadImagesTest();
    new MultilineTest();
    new StaticTextTest();
    new StaticWebViewTest();
    new UIMovieTest();
    new FontTest();
    new WebViewTest();
    new FunctionSignalTest();
    new KeyboardTest();
    new FullscreenTest();
//$UNITTEST_CTOR
}

#include <fstream>
#include <algorithm>

using namespace DAVA;
using namespace DAVA::Net;

void GameCore::OnAppStarted()
{
    testListScreen = new TestListScreen();
    UIScreenManager::Instance()->RegisterScreen(0, testListScreen);

    InitNetwork();
    RunOnlyThisTest();
    RegisterTests();
    RunTests();
}

GameCore::GameCore() 
    : currentScreen(nullptr)
    , testListScreen(nullptr)
{
}

GameCore::~GameCore()
{
}

void GameCore::RegisterScreen(BaseScreen *screen)
{
    UIScreenManager::Instance()->RegisterScreen(screen->GetScreenId(), screen);

    screens.push_back(screen);
    testListScreen->AddTestScreen(screen);
}

void GameCore::ShowStartScreen()
{
    currentScreen = nullptr;
    UIScreenManager::Instance()->SetScreen(0);
}

void GameCore::CreateDocumentsFolder()
{
    FilePath documentsPath = FileSystem::Instance()->GetUserDocumentsPath() + "TestBed/";
    
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
    for(auto testScreen : screens)
    {
        SafeRelease(testScreen);
    }
    screens.clear();
    
    SafeRelease(testListScreen);
    netLogger.Uninstall();
}

void GameCore::BeginFrame()
{
    ApplicationCore::BeginFrame();
}

void GameCore::RunTests()
{
    if ("" != runOnlyThisTest)
    {
        for (auto screen : screens)
        {
            if (!IsNeedSkipTest(*screen))
            {
                currentScreen = screen;
            }
        }
    }
    else
    {
        currentScreen = nullptr;
    }

    if (nullptr != currentScreen)
    {
        UIScreenManager::Instance()->SetScreen(currentScreen->GetScreenId());
    }
    else
    {
        UIScreenManager::Instance()->SetScreen(0);
    }
}

bool GameCore::IsNeedSkipTest(const BaseScreen& screen) const
{
    if (runOnlyThisTest.empty())
    {
        return false;
    }

    const String& name = screen.GetName();

    return 0 != CompareCaseInsensitive(runOnlyThisTest, name);
}

const char8 GameCore::announceMulticastGroup[] = "239.192.100.1";

void GameCore::InitNetwork()
{
    enum eServiceTypes
    {
        SERVICE_LOG = 0,
        SERVICE_MEMPROF = 1
    };

    auto loggerCreate = [this](uint32 serviceId, void*) -> IChannelListener* {
        if (!loggerInUse)
        {
            loggerInUse = true;
            return &netLogger;
        }
        return nullptr;
    };
    NetCore::Instance()->RegisterService(SERVICE_LOG, loggerCreate,
                                         [this](IChannelListener* obj, void*) -> void { loggerInUse = false; });

#if defined(DAVA_MEMORY_PROFILING_ENABLE)
    auto memprofCreate = [this](uint32 serviceId, void*) -> IChannelListener* {
        if (!memprofInUse)
        {
            memprofInUse = true;
            return &memprofServer;
        }
        return nullptr;
    };
    NetCore::Instance()->RegisterService(SERVICE_MEMPROF, memprofCreate,
                                         [this](IChannelListener* obj, void*) -> void { memprofInUse = false; });
#endif

    NetConfig config(SERVER_ROLE);
    config.AddTransport(TRANSPORT_TCP, Net::Endpoint(9999));
    config.AddService(SERVICE_LOG);
#if defined(DAVA_MEMORY_PROFILING_ENABLE)
    config.AddService(SERVICE_MEMPROF);
#endif

    peerDescr = PeerDescription(config);

    Net::Endpoint annoEndpoint(announceMulticastGroup, ANNOUNCE_PORT);
    id_anno = NetCore::Instance()->CreateAnnouncer(annoEndpoint, ANNOUNCE_TIME_PERIOD, MakeFunction(this, &GameCore::AnnounceDataSupplier));
    id_net = NetCore::Instance()->CreateController(config, NULL);
}

size_t GameCore::AnnounceDataSupplier(size_t length, void* buffer)
{
    if (true == peerDescr.NetworkInterfaces().empty())
    {
        peerDescr.SetNetworkInterfaces(NetCore::Instance()->InstalledInterfaces());
        if (true == peerDescr.NetworkInterfaces().empty())
            return 0;
    }
    return peerDescr.Serialize(buffer, length);
}
