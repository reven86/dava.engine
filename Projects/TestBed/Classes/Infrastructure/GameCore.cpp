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
#include "Tests/UIBackgroundTest.h"
#include "Tests/ClipTest.h"
#include "Tests/InputTest.h"
#include "Tests/FloatingPointExceptionTest.h"
#include "Tests/DlcTest.h"
#include "Tests/CoreTest.h"
#include "Tests/FormatsTest.h"
#include "Tests/GPUTest.h"
#include "Tests/PackManagerTest.h"
//$UNITTEST_INCLUDE

#if defined(DAVA_MEMORY_PROFILING_ENABLE)
#include "MemoryManager/MemoryProfiler.h"
#endif

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
    new DlcTest();
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
    new UIBackgroundTest();
    new ClipTest();
    new InputTest();
    new CoreTest();
    new FormatsTest();
    new GPUTest();
    new FloatingPointExceptionTest();
    new PackManagerTest();
    //$UNITTEST_CTOR
}

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

void GameCore::RegisterScreen(BaseScreen* screen)
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

File* GameCore::CreateDocumentsFile(const String& filePathname)
{
    FilePath workingFilepathname = FilePath::FilepathInDocuments(filePathname);

    FileSystem::Instance()->CreateDirectory(workingFilepathname.GetDirectory(), true);

    File* retFile = File::Create(workingFilepathname, File::CREATE | File::WRITE);
    return retFile;
}

void GameCore::OnAppFinished()
{
    for (auto testScreen : screens)
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

    const FastName& name = screen.GetName();

    return 0 != CompareCaseInsensitive(runOnlyThisTest, name.c_str());
}

void GameCore::InitNetwork()
{
    auto loggerCreate = [this](uint32 serviceId, void*) -> IChannelListener* {
        if (!loggerInUse)
        {
            loggerInUse = true;
            return &netLogger;
        }
        return nullptr;
    };
    NetCore::Instance()->RegisterService(NetCore::SERVICE_LOG, loggerCreate,
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
    NetCore::Instance()->RegisterService(NetCore::SERVICE_MEMPROF, memprofCreate,
                                         [this](IChannelListener* obj, void*) -> void { memprofInUse = false; });
#endif

    eNetworkRole role = SERVER_ROLE;
    Net::Endpoint endpoint = Net::Endpoint(NetCore::DEFAULT_TCP_PORT);

    NetConfig config(role);
    config.AddTransport(TRANSPORT_TCP, endpoint);
    config.AddService(NetCore::SERVICE_LOG);
#if defined(DAVA_MEMORY_PROFILING_ENABLE)
    config.AddService(NetCore::SERVICE_MEMPROF);
#endif
    peerDescr = PeerDescription(config);
    Net::Endpoint annoUdpEndpoint(NetCore::defaultAnnounceMulticastGroup, NetCore::DEFAULT_UDP_ANNOUNCE_PORT);
    Net::Endpoint annoTcpEndpoint(NetCore::DEFAULT_TCP_ANNOUNCE_PORT);
    id_anno = NetCore::Instance()->CreateAnnouncer(annoUdpEndpoint, DEFAULT_ANNOUNCE_TIME_PERIOD, MakeFunction(this, &GameCore::AnnounceDataSupplier), annoTcpEndpoint);
    id_net = NetCore::Instance()->CreateController(config, nullptr);
}

size_t GameCore::AnnounceDataSupplier(size_t length, void* buffer)
{
    if (true == peerDescr.NetworkInterfaces().empty())
    {
        peerDescr.SetNetworkInterfaces(NetCore::Instance()->InstalledInterfaces());
    }
    return peerDescr.Serialize(buffer, length);
}
