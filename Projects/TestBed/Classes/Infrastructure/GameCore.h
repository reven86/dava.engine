#pragma once

#include "Core/ApplicationCore.h"
#include "Core/Core.h"

#include "Network/NetCore.h"
#include "Network/PeerDesription.h"
#include "Network/Services/NetLogger.h"

#if defined(DAVA_MEMORY_PROFILING_ENABLE)
#include "Network/Services/MMNet/MMNetServer.h"
#endif

namespace DAVA
{
class Engine;
class Window;
}

class TestData;
class BaseScreen;
class TestListScreen;

class GameCore
{
    struct ErrorData
    {
        DAVA::int32 line;
        DAVA::String command;
        DAVA::String filename;
        DAVA::String testName;
        DAVA::String testMessage;
    };

public:
    GameCore(DAVA::Engine& engine);

    DAVA::Engine& GetEngine() const;

    void OnGameLoopStarted();
    void OnGameLoopStopped();
    void OnEngineCleanup();

    void OnWindowSizeChanged(DAVA::Window& w, DAVA::float32 width, DAVA::float32 height, DAVA::float32 scaleX, DAVA::float32 scaleY);
    void OnWindowCreated(DAVA::Window& w);
    void OnWindowDestroyed(DAVA::Window& w);

    void OnSuspended();
    void OnResumed();

    void OnUpdateConsole(DAVA::float32 frameDelta);

    void RegisterScreen(BaseScreen* screen);
    void ShowStartScreen();

private:
    void RegisterTests();
    void RunTests();

    void CreateDocumentsFolder();
    DAVA::File* CreateDocumentsFile(const DAVA::String& filePathname);

    void RunOnlyThisTest();
    void OnError();
    bool IsNeedSkipTest(const BaseScreen& screen) const;

    DAVA::Engine& engine;

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

inline DAVA::Engine& GameCore::GetEngine() const
{
    return engine;
}
