#include "Autotesting/AutotestingSystem.h"

#ifdef __DAVAENGINE_AUTOTESTING__

#include "Core/Core.h"
#include "Render/RenderHelper.h"
#include "FileSystem/FileList.h"
#include "Platform/DeviceInfo.h"
#include "Platform/DateTime.h"
#include "FileSystem/KeyedArchive.h"

#include "Autotesting/AutotestingSystemLua.h"
#include "Autotesting/AutotestingDB.h"

#include "Job/JobManager.h"

namespace DAVA
{
AutotestingSystem::AutotestingSystem()
    : luaSystem(nullptr)
    , startTimeMS(0)
    , isInit(false)
    , isRunning(false)
    , needExitApp(false)
    , timeBeforeExit(0.0f)
    , projectName("")
    , groupName("default")
    , deviceName("not-initialized")
    , testsDate("not_found")
    , runId("not_found")
    , testIndex(0)
    , stepIndex(0)
    , logIndex(0)
    , testDescription("")
    , testFileName("")
    , testFilePath("")
    , buildDate("not_found")
    , buildId("zero-build")
    , branch("branch")
    , framework("framework")
    , branchRev("0")
    , frameworkRev("0")
    , isDB(true)
    , needClearGroupInDB(false)
    , isMaster(true)
    , requestedHelpers(0)
    , masterId("")
    , masterTask("")
    , masterRunId(0)
    , isRegistered(false)
    , isWaiting(false)
    , isInitMultiplayer(false)
    , multiplayerName("")
    , waitTimeLeft(0.0f)
    , waitCheckTimeLeft(0.0f)
{
    new AutotestingDB();
}

AutotestingSystem::~AutotestingSystem()
{
    SafeRelease(luaSystem);
    if (AutotestingDB::Instance())
        AutotestingDB::Instance()->Release();

    SafeRelease(screenShotTexture);
}

void AutotestingSystem::InitLua(AutotestingSystemLuaDelegate* _delegate)
{
    Logger::Info("AutotestingSystem::InitLua");
    DVASSERT(nullptr == luaSystem);
    luaSystem = new AutotestingSystemLua();
    luaSystem->SetDelegate(_delegate);
}

String AutotestingSystem::ResolvePathToAutomation(const String& automationPath)
{
    Logger::Info("AutotestingSystem::ResolvePathToAutomation platform=%s path=%s", DeviceInfo::GetPlatformString().c_str(), automationPath.c_str());
    String automationResolvedStrPath;
    // Try to find automation data in Documents
    if (DeviceInfo::GetPlatform() == DeviceInfo::PLATFORM_PHONE_WIN_UAP)
    {
        //TODO: it's temporary solution will be changed with upgrading WinSDK and launching tool
        automationResolvedStrPath = "d:" + automationPath;
    }
    else if (DeviceInfo::GetPlatform() == DeviceInfo::PLATFORM_ANDROID)
    {
        automationResolvedStrPath = FileSystem::Instance()->GetPublicDocumentsPath().GetAbsolutePathname() + automationPath;
    }
    else
    {
        automationResolvedStrPath = "~doc:" + automationPath;
    }

    if (FilePath(automationResolvedStrPath).Exists())
    {
        Logger::Info("AutotestingSystem::ResolvePathToAutomation resolved path=%s", automationResolvedStrPath.c_str());
        return automationResolvedStrPath;
    }

    // If there are no automation data in documents, try to find it in Data
    if (FilePath("~res:" + automationPath).Exists())
    {
        automationResolvedStrPath = "~res:" + automationPath;
        Logger::Info("AutotestingSystem::ResolvePathToAutomation resolved path=%s", automationResolvedStrPath.c_str());
        return automationResolvedStrPath;
    }
    return "";
}

// This method is called on application started and it handle autotest initialisation
void AutotestingSystem::OnAppStarted()
{
    Logger::Info("AutotestingSystem::OnAppStarted");

    if (isInit)
    {
        Logger::Error("AutotestingSystem::OnAppStarted App already initialized.");
        return;
    }
    deviceName = AutotestingSystemLua::Instance()->GetDeviceName();
    FetchParametersFromIdYaml();

    if (isDB)
    {
        SetUpConnectionToDB();
        FetchParametersFromDB();
    }

    const String testFileLocation = Format("/Autotesting/Tests/%s/%s.lua", groupName.c_str(), testFileName.c_str());
    String testFileStrPath = ResolvePathToAutomation(testFileLocation);
    if (testFileStrPath.empty())
    {
        Logger::Error("AutotestingSystemLua::OnAppStarted: couldn't open %s", testFileLocation.c_str());
        return;
    }

    AutotestingDB::Instance()->WriteLogHeader();

    AutotestingSystemLua::Instance()->InitFromFile(testFileStrPath);

    Size2i size = VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize();

    Texture::FBODescriptor desc;
    desc.width = uint32(size.dx);
    desc.height = uint32(size.dy);
    desc.format = FORMAT_RGBA8888;
    desc.needDepth = true;
    desc.needPixelReadback = true;

    screenShotTexture = Texture::CreateFBO(desc);
}

void AutotestingSystem::OnAppFinished()
{
    Logger::Info("AutotestingSystem::OnAppFinished in");
    ExitApp();
    Logger::Info("AutotestingSystem::OnAppFinished out");
}

void AutotestingSystem::RunTests()
{
    if (!isInit || isRunning)
    {
        return;
    }
    isRunning = true;
    OnTestStarted();
}

void AutotestingSystem::OnInit()
{
    DVASSERT(!isInit);
    isInit = true;
}

// Get test parameters from id.yaml
void AutotestingSystem::FetchParametersFromIdYaml()
{
    Logger::Info("AutotestingSystem::FetchParametersFromIdYaml");
    RefPtr<KeyedArchive> option = GetIdYamlOptions();

    buildId = option->GetString("BuildId");
    buildDate = option->GetString("Date");
    branch = option->GetString("Branch");
    framework = option->GetString("Framework");
    branchRev = option->GetString("BranchRev");
    frameworkRev = option->GetString("FrameworkRev");

    // Check is build fol local debugging.  By default: use DB.
    bool isLocalBuild = option->GetBool("LocalBuild", false);
    if (isLocalBuild)
    {
        groupName = option->GetString("Group", AutotestingDB::DB_ERROR_STR_VALUE);
        testFileName = option->GetString("Filename", AutotestingDB::DB_ERROR_STR_VALUE);
        isDB = false;
    }
}

RefPtr<KeyedArchive> AutotestingSystem::GetIdYamlOptions()
{
    const String idYamlStrLocation = "/Autotesting/id.yaml";
    String idYamlStrPath = ResolvePathToAutomation(idYamlStrLocation);
    RefPtr<KeyedArchive> option(new KeyedArchive());
    if (idYamlStrPath.empty() || !option->LoadFromYamlFile(idYamlStrPath))
    {
        ForceQuit("Couldn't open file " + idYamlStrLocation);
    }

    return option;
}

// Get test parameters from autotesting db
void AutotestingSystem::FetchParametersFromDB()
{
    Logger::Info("AutotestingSystem::FetchParametersFromDB");
    AutotestingDB* db = AutotestingDB::Instance();
    groupName = db->GetStringTestParameter(deviceName, "Group");
    if (groupName == AutotestingDB::DB_ERROR_STR_VALUE)
    {
        ForceQuit("Couldn't get 'Group' parameter from DB.");
    }
    testFileName = db->GetStringTestParameter(deviceName, "Filename");
    if (groupName == AutotestingDB::DB_ERROR_STR_VALUE)
    {
        ForceQuit("Couldn't get 'Filename' parameter from DB.");
    }
    runId = db->GetStringTestParameter(deviceName, "RunId");
    if (runId == AutotestingDB::DB_ERROR_STR_VALUE)
    {
        ForceQuit("Couldn't get 'RunId' parameter from DB.");
    }
    testIndex = db->GetIntTestParameter(deviceName, "TestIndex");
    if (testIndex == AutotestingDB::DB_ERROR_INT_VALUE)
    {
        ForceQuit("Couldn't get TestIndex parameter from DB.");
    }
}

// Read DB parameters from config file and set connection to it
void AutotestingSystem::SetUpConnectionToDB()
{
    const String dbConfigLocation = "/Autotesting/dbConfig.yaml";
    String dbConfigStrPath = ResolvePathToAutomation(dbConfigLocation);
    KeyedArchive* option = new KeyedArchive();
    if (dbConfigStrPath.empty() || !option->LoadFromYamlFile(dbConfigStrPath))
    {
        ForceQuit("Couldn't open file " + dbConfigLocation);
    }

    String dbName = option->GetString("name");
    String dbAddress = option->GetString("hostname");
    String collection = option->GetString("collection");
    int32 dbPort = option->GetInt32("port");
    Logger::Info("AutotestingSystem::SetUpConnectionToDB %s -> %s[%s:%d]", collection.c_str(), dbName.c_str(), dbAddress.c_str(), dbPort);

    if (!AutotestingDB::Instance()->ConnectToDB(collection, dbName, dbAddress, dbPort))
    {
        ForceQuit("Couldn't connect to Test DB");
    }

    SafeRelease(option);
}

// Multiplayer API
void AutotestingSystem::InitializeDevice()
{
    Logger::Info("AutotestingSystem::InitializeDevice");
    if (!isDB)
    {
        OnError("Couldn't use multiplayer test in local mode.");
    }
    isInitMultiplayer = true;
}

String AutotestingSystem::GetCurrentTimeString()
{
    DateTime time = DateTime::Now();
    return Format("%02d-%02d-%02d", time.GetHour(), time.GetMinute(), time.GetSecond());
}

void AutotestingSystem::OnTestStart(const String& testDescription)
{
    Logger::Info("AutotestingSystem::OnTestStart %s", testDescription.c_str());
    AutotestingDB::Instance()->Log("DEBUG", Format("OnTestStart %s", testDescription.c_str()));
    if (isDB)
        AutotestingDB::Instance()->SetTestStarted();
}

void AutotestingSystem::OnStepStart(const String& stepName)
{
    Logger::Info("AutotestingSystem::OnStepStart %s", stepName.c_str());

    OnStepFinished();

    AutotestingDB::Instance()->Log("INFO", stepName);
}

void AutotestingSystem::OnStepFinished()
{
    Logger::Info("AutotestingSystem::OnStepFinished");
    AutotestingDB::Instance()->Log("INFO", "Success");
}

void AutotestingSystem::Update(float32 timeElapsed)
{
    if (!isInit)
    {
        return;
    }
    if (needExitApp)
    {
        timeBeforeExit -= timeElapsed;
        if (timeBeforeExit <= 0.0f)
        {
            needExitApp = false;
            String server = AutotestingDB::Instance()->GetStringTestParameter(deviceName, "Server");
            if (server != AutotestingDB::DB_ERROR_STR_VALUE)
            {
                AutotestingSystemLua::Instance()->SetServerQueueState(server, 0);
            }
            JobManager::Instance()->WaitWorkerJobs();
#if !defined(__DAVAENGINE_COREV2__)
            Core::Instance()->Quit();
#endif
        }
        return;
    }

    if (isRunning)
    {
        luaSystem->Update(timeElapsed);
    }
}

void AutotestingSystem::Draw()
{
    if (!isInit)
    {
        return;
    }
    if (!touches.empty())
    {
        for (Map<int32, UIEvent>::iterator it = touches.begin(); it != touches.end(); ++it)
        {
            Vector2 point = it->second.point;
            RenderSystem2D::Instance()->DrawCircle(point, 25.0f, Color::White);
        }
    }
    RenderSystem2D::Instance()->DrawCircle(GetMousePosition(), 15.0f, Color::White);
}

void AutotestingSystem::OnTestStarted()
{
    Logger::Info("AutotestingSystem::OnTestsStarted");
    startTimeMS = SystemTimer::Instance()->FrameStampTimeMS();
    luaSystem->StartTest();
}

void AutotestingSystem::OnError(const String& errorMessage)
{
    Logger::Error("AutotestingSystem::OnError %s", errorMessage.c_str());

    AutotestingDB::Instance()->Log("ERROR", errorMessage);

    MakeScreenShot();

    AutotestingDB::Instance()->Log("ERROR", screenShotName);

    if (isDB && isInitMultiplayer)
    {
        AutotestingDB::Instance()->WriteState(deviceName, "State", "error");
    }

    ExitApp();
}

void AutotestingSystem::ForceQuit(const String& errorMessage)
{
    DVASSERT(false, errorMessage.c_str());
#if !defined(__DAVAENGINE_COREV2__)
    Core::Instance()->Quit();
#endif
}

void AutotestingSystem::MakeScreenShot()
{
    Logger::Info("AutotestingSystem::MakeScreenShot");
    String currentDateTime = GetCurrentTimeString();
    screenShotName = Format("%s_%s_%s_%d_%s", groupName.c_str(), testFileName.c_str(), runId.c_str(), testIndex, currentDateTime.c_str());
    String log = Format("AutotestingSystem::ScreenShotName %s", screenShotName.c_str());
    AutotestingDB::Instance()->Log("INFO", log.c_str());

    UIScreen* currentScreen = UIControlSystem::Instance()->GetScreen();
    if (currentScreen)
    {
        const Size2i& size = VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize();

        rhi::Viewport viewport;
        viewport.x = viewport.y = 0;
        viewport.width = size.dx;
        viewport.height = size.dy;
        UIControlSystem::Instance()->GetScreenshoter()->MakeScreenshot(currentScreen, screenShotTexture, MakeFunction(this, &AutotestingSystem::OnScreenShot), true, false, viewport);
    }
    else
    {
        Logger::Error("AutotestingSystem::MakeScreenShot no current screen");
    }
}

const String& AutotestingSystem::GetScreenShotName()
{
    Logger::Info("AutotestingSystem::GetScreenShotName %s", screenShotName.c_str());
    return screenShotName;
}

void AutotestingSystem::OnScreenShot(Texture* texture)
{
    Function<void()> fn = Bind(&AutotestingSystem::OnScreenShotInternal, this, texture);
    JobManager::Instance()->CreateWorkerJob(fn);
    isScreenShotSaving = true;
}

void AutotestingSystem::OnScreenShotInternal(Texture* texture)
{
    DVASSERT(texture);

    Logger::Info("AutotestingSystem::OnScreenShot %s", screenShotName.c_str());
    uint64 startTime = SystemTimer::Instance()->AbsoluteMS();

    DAVA::ScopedPtr<DAVA::Image> image(texture->CreateImageFromMemory());
    const Size2i& size = VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize();
    image->ResizeCanvas(uint32(size.dx), uint32(size.dy));
    image->Save(FilePath(AutotestingDB::Instance()->logsFolder + Format("/%s.png", screenShotName.c_str())));

    uint64 finishTime = SystemTimer::Instance()->AbsoluteMS();
    Logger::FrameworkDebug("AutotestingSystem::OnScreenShot Upload: %d", finishTime - startTime);
    isScreenShotSaving = false;
}

void AutotestingSystem::ClickSystemBack()
{
    Logger::Info("AutotestingSystem::ClickSystemBack");
    UIEvent keyEvent;
    keyEvent.device = UIEvent::Device::KEYBOARD;
    keyEvent.phase = DAVA::UIEvent::Phase::KEY_DOWN;
    keyEvent.key = DAVA::Key::BACK;
    keyEvent.timestamp = (SystemTimer::FrameStampTimeMS() / 1000.0);
    UIControlSystem::Instance()->OnInput(&keyEvent);
}

void AutotestingSystem::OnTestsFinished()
{
    Logger::Info("AutotestingSystem::OnTestsFinished");

    // Mark last step as SUCCESS
    OnStepFinished();

    if (isDB && isInitMultiplayer)
    {
        AutotestingDB::Instance()->WriteState(deviceName, "State", "finished");
    }

    // Mark test as SUCCESS
    AutotestingDB::Instance()->Log("INFO", "Test finished.");

    ExitApp();
}

void AutotestingSystem::OnTestSkipped()
{
    Logger::Info("AutotestingSystem::OnTestSkipped");

    if (isDB && isInitMultiplayer)
    {
        AutotestingDB::Instance()->WriteState(deviceName, "State", "skipped");
    }

    // Mark test as SKIPPED
    AutotestingDB::Instance()->Log("INFO", "Test skipped.");

    ExitApp();
}

void AutotestingSystem::OnInput(const UIEvent& input)
{
    if (UIScreenManager::Instance())
    {
        String screenName = (UIScreenManager::Instance()->GetScreen()) ? UIScreenManager::Instance()->GetScreen()->GetName().c_str() : "noname";
        Logger::Info("AutotestingSystem::OnInput screen is %s (%d)", screenName.c_str(), UIScreenManager::Instance()->GetScreenId());
    }

    int32 id = input.touchId;
    switch (input.phase)
    {
    case UIEvent::Phase::BEGAN:
    {
        mouseMove = input;
        if (!IsTouchDown(id))
        {
            touches[id] = input;
        }
        else
        {
            Logger::Error("AutotestingSystemYaml::OnInput PHASE_BEGAN duplicate touch id=%d", id);
        }
    }
    break;
#if !defined(__DAVAENGINE_IPHONE__) && !defined(__DAVAENGINE_ANDROID__)
    case UIEvent::Phase::MOVE:
    {
        mouseMove = input;
        if (IsTouchDown(id))
        {
            Logger::Error("AutotestingSystemYaml::OnInput PHASE_MOVE id=%d must be PHASE_DRAG", id);
        }
    }
    break;
#endif
    case UIEvent::Phase::DRAG:
    {
        mouseMove = input;
        Map<int32, UIEvent>::iterator findIt = touches.find(id);
        if (findIt != touches.end())
        {
            findIt->second = input;
        }
        else
        {
            Logger::Error("AutotestingSystemYaml::OnInput PHASE_DRAG id=%d must be PHASE_MOVE", id);
        }
    }
    break;
    case UIEvent::Phase::ENDED:
    {
        mouseMove = input;
        Map<int32, UIEvent>::iterator findIt = touches.find(id);
        if (findIt != touches.end())
        {
            touches.erase(findIt);
        }
        else
        {
            Logger::Error("AutotestingSystemYaml::OnInput PHASE_ENDED id=%d not found", id);
        }
    }
    break;
    default:
        //TODO: keyboard input
        break;
    }
}

bool AutotestingSystem::FindTouch(int32 id, UIEvent& touch)
{
    bool isFound = false;
    Map<int32, UIEvent>::iterator findIt = touches.find(id);
    if (findIt != touches.end())
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
    if (needExitApp)
    {
        return;
    }
    isRunning = false;
    isWaiting = false;
    needExitApp = true;
    timeBeforeExit = 1.0f;
}

// Multiplayer API

// Working with DB api
};

#endif //__DAVAENGINE_AUTOTESTING__
