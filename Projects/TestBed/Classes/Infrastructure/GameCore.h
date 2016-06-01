#ifndef __GAMECORE_H__
#define __GAMECORE_H__

#include "Core/ApplicationCore.h"
#include "Core/Core.h"

#include "Network/NetCore.h"
#include "Network/PeerDesription.h"
#include "Network/Services/NetLogger.h"

#if defined(DAVA_MEMORY_PROFILING_ENABLE)
#include "Network/Services/MMNet/MMNetServer.h"
#endif

#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Engine.h"

class TestData;
class BaseScreen;
class TestListScreen;

class GameCore final
{
public:
    GameCore(DAVA::Engine* eng);
    ~GameCore() = default;

    static GameCore* pthis;
    static GameCore* Instance()
    {
        return pthis;
    };

    void OnGameLoopStarted();
    void OnGameLoopStopped();

    void OnWindowSizeChanged(DAVA::Window* w, DAVA::float32 width, DAVA::float32 height, DAVA::float32 scaleX, DAVA::float32 scaleY);

    void RegisterScreen(BaseScreen* screen);
    void ShowStartScreen();

    void OnUpdateConsole(DAVA::float32 frameDelta);

protected:
    void RegisterTests();
    void RunTests();

    void CreateDocumentsFolder();
    DAVA::File* CreateDocumentsFile(const DAVA::String& filePathname);

private:
    void RunOnlyThisTest();
    void OnError();
    bool IsNeedSkipTest(const BaseScreen& screen) const;

    DAVA::Engine* engine = nullptr;
    DAVA::String runOnlyThisTest;

    BaseScreen* currentScreen;
    TestListScreen* testListScreen;

    DAVA::Vector<BaseScreen*> screens;
};

#else

class TestData;
class BaseScreen;
class TestListScreen;
class GameCore : public DAVA::ApplicationCore
{
    struct ErrorData
    {
        DAVA::int32 line;
        DAVA::String command;
        DAVA::String filename;
        DAVA::String testName;
        DAVA::String testMessage;
    };

protected:
    virtual ~GameCore();

public:
    GameCore();

    static GameCore* Instance()
    {
        return (GameCore*)DAVA::Core::GetApplicationCore();
    };

    void OnAppStarted() override;
    void OnAppFinished() override;

    void BeginFrame() override;

    void RegisterScreen(BaseScreen* screen);
    void ShowStartScreen();

protected:
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    void OnBackground() override{};

    void OnForeground() override{};

    void OnDeviceLocked() override{};
#endif //#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)

    void RegisterTests();
    void RunTests();

    void CreateDocumentsFolder();
    DAVA::File* CreateDocumentsFile(const DAVA::String& filePathname);

private:
    void RunOnlyThisTest();
    void OnError();
    bool IsNeedSkipTest(const BaseScreen& screen) const;

    DAVA::String runOnlyThisTest;

    BaseScreen* currentScreen;
    TestListScreen* testListScreen;

    DAVA::Vector<BaseScreen*> screens;

    // Network support
    void InitNetwork();

    size_t AnnounceDataSupplier(size_t length, void* buffer);

    DAVA::Net::NetCore::TrackId id_anno = DAVA::Net::NetCore::INVALID_TRACK_ID;
    DAVA::Net::NetCore::TrackId id_net = DAVA::Net::NetCore::INVALID_TRACK_ID;

    DAVA::Net::NetLogger netLogger;
#if defined(DAVA_MEMORY_PROFILING_ENABLE)
    DAVA::Net::MMNetServer memprofServer;
    bool memprofInUse = false;
#endif
    DAVA::Net::PeerDescription peerDescr;

    bool loggerInUse = false;
};

#endif // __DAVAENGINE_COREV2__

#endif // __GAMECORE_H__
