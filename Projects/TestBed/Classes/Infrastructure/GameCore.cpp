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
//$UNITTEST_INCLUDE

#if defined(DAVA_MEMORY_PROFILING_ENABLE)
#include "MemoryManager/MemoryProfiler.h"
#endif

#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Engine.h"

int GameMain(DAVA::Vector<DAVA::String> cmdline)
{
    KeyedArchive* appOptions = new KeyedArchive();
    appOptions->SetString("title", "TestBed");
    appOptions->SetInt32("renderer", rhi::RHI_DX11);
    appOptions->SetInt32("fullscreen", 0);
    appOptions->SetInt32("bpp", 32);
    appOptions->SetInt32("rhi_threaded_frame_count", 2);
    appOptions->SetInt32("width", 1024);
    appOptions->SetInt32("height", 768);

    bool console = false;
    if (cmdline.size() > 1)
    {
        console = cmdline[1] == "--console";
    }

    Vector<String> modules;
    DAVA::Engine e;
    e.SetOptions(appOptions);
    e.Init(console, modules);

    GameCore game(&e);
    return e.Run();
}

GameCore* GameCore::pthis = nullptr;

GameCore::GameCore(Engine* eng)
    : engine(eng)
    , currentScreen(nullptr)
    , testListScreen(nullptr)
{
    pthis = this;

    engine->signalGameLoopStarted.Connect(this, &GameCore::OnGameLoopStarted);
    engine->signalGameLoopStopped.Connect(this, &GameCore::OnGameLoopStopped);

    if (engine->IsConsoleMode())
        engine->signalUpdate.Connect(this, &GameCore::OnUpdateConsole);
    else
        engine->PrimaryWindow()->signalSizeScaleChanged.Connect(this, &GameCore::OnWindowSizeChanged);
}

void GameCore::RunOnlyThisTest()
{
    //runOnlyThisTest = "UIScrollViewTest";
}

void GameCore::OnGameLoopStarted()
{
    Logger::Debug("****** GameCore::OnGameLoopStarted");

    if (!engine->IsConsoleMode())
    {
        testListScreen = new TestListScreen();
        UIScreenManager::Instance()->RegisterScreen(0, testListScreen);
        RegisterTests();
        RunTests();

        engine->PrimaryWindow()->Resize(1024, 768);
    }

    engine->RunAsyncOnMainThread([]() {
        Logger::Error("******** KABOOM on main thread********");
    });

    if (!engine->IsConsoleMode())
    {
        engine->PrimaryWindow()->RunAsyncOnUIThread([]() {
            Logger::Error("******** KABOOM on UI thread********");
        });
    }
}

void GameCore::OnGameLoopStopped()
{
    Logger::Debug("****** GameCore::OnGameLoopStopped");

    for (auto testScreen : screens)
    {
        SafeRelease(testScreen);
    }
    screens.clear();
    SafeRelease(testListScreen);
}

void GameCore::OnWindowSizeChanged(DAVA::Window* w, DAVA::float32 width, DAVA::float32 height, DAVA::float32 scaleX, DAVA::float32 scaleY)
{
    Logger::Debug("********** GameCore::OnWindowSizeChanged: w=%.1f, h==%.1f, sx=%.1f, sy=%.1f", width, height, scaleX, scaleY);
}

void GameCore::OnUpdateConsole(DAVA::float32 frameDelta)
{
    static int frameCount = 0;
    frameCount += 1;
    Logger::Debug("****** update: count=%d, delta=%f", frameCount, frameDelta);
    if (frameCount >= 100)
    {
        Logger::Debug("****** quit");
        engine->Quit();
    }
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
#if !defined(__DAVAENGINE_COREV2__)
    new FullscreenTest();
#endif
    new UIBackgroundTest();
    new ClipTest();
    new InputTest();
    new CoreTest();
    new FormatsTest();
    new FloatingPointExceptionTest();
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

void GameCore::CreateDocumentsFolder()
{
    FilePath documentsPath = FileSystem::Instance()->GetUserDocumentsPath() + "TestBed/";
    FileSystem::Instance()->CreateDirectory(documentsPath, true);
    FileSystem::Instance()->SetCurrentDocumentsDirectory(documentsPath);
}

DAVA::File* GameCore::CreateDocumentsFile(const DAVA::String& filePathname)
{
    FilePath workingFilepathname = FilePath::FilepathInDocuments(filePathname);
    FileSystem::Instance()->CreateDirectory(workingFilepathname.GetDirectory(), true);
    File* retFile = File::Create(workingFilepathname, File::CREATE | File::WRITE);
    return retFile;
}

void GameCore::OnError()
{
    DavaDebugBreak();
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

#else

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
    new FloatingPointExceptionTest();
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

#endif // defined(__DAVAENGINE_COREV2__)
