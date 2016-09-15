#include "Infrastructure/GameCore.h"

#include "Engine/EngineModule.h"

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
#include "Tests/MicroWebBrowserTest.h"
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
#include "Tests/AssertTest.h"
#include "Tests/CoreV2Test.h"
//$UNITTEST_INCLUDE

#if defined(DAVA_MEMORY_PROFILING_ENABLE)
#include "MemoryManager/MemoryProfiler.h"
#endif

using namespace DAVA;
using namespace DAVA::Net;

int GameMain(DAVA::Vector<DAVA::String> cmdline)
{
    KeyedArchive* appOptions = new KeyedArchive();
    appOptions->SetString("title", "TestBed");
    appOptions->SetInt32("fullscreen", 0);
    appOptions->SetInt32("bpp", 32);
    appOptions->SetInt32("rhi_threaded_frame_count", 2);
    appOptions->SetInt32("width", 1024);
    appOptions->SetInt32("height", 768);
#if defined(__DAVAENGINE_QT__)
    appOptions->SetInt32("renderer", rhi::RHI_GLES2);
#elif defined(__DAVAENGINE_MACOS__)
    appOptions->SetInt32("renderer", rhi::RHI_GLES2);
#elif defined(__DAVAENGINE_IPHONE__)
    appOptions->SetInt32("renderer", rhi::RHI_GLES2);
#elif defined(__DAVAENGINE_WIN32__)
    appOptions->SetInt32("renderer", rhi::RHI_DX9);
#elif defined(__DAVAENGINE_WIN_UAP__)
    appOptions->SetInt32("renderer", rhi::RHI_DX11);
#elif defined(__DAVAENGINE_ANDROID__)
    appOptions->SetInt32("renderer", rhi::RHI_GLES2);
#endif

    eEngineRunMode runmode = eEngineRunMode::GUI_STANDALONE;
    if (cmdline.size() > 1 && cmdline[1] == "--console")
    {
        runmode = eEngineRunMode::CONSOLE_MODE;
    }

    Vector<String> modules = {
        "JobManager",
        "NetCore",
        "LocalizationSystem",
        "SoundSystem",
        "DownloadManager",
    };

    DAVA::Engine e;
    e.SetOptions(appOptions);
    e.Init(runmode, modules);

    GameCore game(&e);
    return e.Run();
}

GameCore::GameCore(Engine* e)
    : engine(e)
    , currentScreen(nullptr)
    , testListScreen(nullptr)
{
    engine->gameLoopStarted.Connect(this, &GameCore::OnGameLoopStarted);
    engine->gameLoopStopped.Connect(this, &GameCore::OnGameLoopStopped);
    engine->beforeTerminate.Connect(this, &GameCore::OnBeforeTerminate);

    engine->suspended.Connect(this, &GameCore::OnSuspended);
    engine->resumed.Connect(this, &GameCore::OnResumed);

    if (engine->IsConsoleMode())
    {
        engine->update.Connect(this, &GameCore::OnUpdateConsole);
    }
    else
    {
        engine->windowCreated.Connect(this, &GameCore::OnWindowCreated);
        engine->windowDestroyed.Connect(this, &GameCore::OnWindowDestroyed);

        Window* w = engine->PrimaryWindow();
        w->sizeScaleChanged.Connect(this, &GameCore::OnWindowSizeChanged);
    }

    engine->GetContext()->uiControlSystem->SetClearColor(Color::Black);
}

void GameCore::OnGameLoopStarted()
{
    Logger::Debug("****** GameCore::OnGameLoopStarted");

    InitNetwork();
    RunOnlyThisTest();

    if (engine->IsConsoleMode())
    {
        engine->RunAsyncOnMainThread([]() {
            Logger::Error("******** KABOOM on main thread********");
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

void GameCore::OnBeforeTerminate()
{
    Logger::Debug("****** GameCore::OnBeforeTerminate");
    netLogger.Uninstall();
}

void GameCore::OnWindowCreated(DAVA::Window& w)
{
    Logger::Error("****** GameCore::OnWindowCreated");

    testListScreen = new TestListScreen();
    UIScreenManager::Instance()->RegisterScreen(0, testListScreen);
    RegisterTests();
    RunTests();
}

void GameCore::OnWindowDestroyed(DAVA::Window& w)
{
    Logger::Error("****** GameCore::OnWindowDestroyed");
}

void GameCore::OnWindowSizeChanged(DAVA::Window& w, DAVA::float32 width, DAVA::float32 height, DAVA::float32 scaleX, DAVA::float32 scaleY)
{
    Logger::Debug("********** GameCore::OnWindowSizeChanged: w=%.1f, h=%.1f, sx=%.1f, sy=%.1f", width, height, scaleX, scaleY);
}

void GameCore::OnSuspended()
{
    Logger::Error("****** GameCore::OnSuspended");
}

void GameCore::OnResumed()
{
    Logger::Error("****** GameCore::OnResumed");
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
    new CoreV2Test(this);
    new DlcTest(this);
    new UIScrollViewTest(this);
    new NotificationScreen(this);
    new SpeedLoadImagesTest(this);
    new MultilineTest(this);
    new StaticTextTest(this);
    new StaticWebViewTest(this);
    new MicroWebBrowserTest(this);
    new UIMovieTest(this);
    new FontTest(this);
    new WebViewTest(this);
    new FunctionSignalTest(this);
    new KeyboardTest(this);
    new FullscreenTest(this);
    new UIBackgroundTest(this);
    new ClipTest(this);
    new InputTest(this);
    new GPUTest(this);
    new CoreTest(this);
    new FormatsTest(this);
    new AssertTest(this);
    new FloatingPointExceptionTest(this);
#if !defined(__DAVAENGINE_COREV2__)
    new PackManagerTest(this);
#endif // !__DAVAENGINE_COREV2__
    //$UNITTEST_CTOR
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
