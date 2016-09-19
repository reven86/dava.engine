#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"
#include "Functional/Functional.h"

#include "Engine/EngineTypes.h"
#include "Engine/Private/EnginePrivateFwd.h"

#include "UI/UIEvent.h"

namespace DAVA
{
class KeyedArchive;
namespace Private
{
class EngineBackend final
{
public:
    static EngineBackend* Instance();

    EngineBackend(const Vector<String>& cmdargs);
    ~EngineBackend();

    EngineBackend(const EngineBackend&) = delete;
    EngineBackend& operator=(const EngineBackend&) = delete;

    void EngineCreated(Engine* e);
    void EngineDestroyed();

    //////////////////////////////////////////////////////////////////////////
    eEngineRunMode GetRunMode() const;
    bool IsStandaloneGUIMode() const;
    bool IsEmbeddedGUIMode() const;
    bool IsConsoleMode() const;

    EngineContext* GetEngineContext() const;
    Window* GetPrimaryWindow() const;
    uint32 GetGlobalFrameIndex() const;
    int32 GetExitCode() const;
    const Vector<String>& GetCommandLine() const;

    /// \brief Get command args in form suitable to use as argc and argv
    ///
    /// Some platforms want command line in form argc and argv, e.g. iOS's UIApplicationMain, Qt's QApplication, etc.
    /// EngineBackend grabs command line at program start and stores it as DAVA::Vector of DAVA::strings, so GetCommandLineAsArgv allows
    /// passing command line arguments to those functions.
    ///
    /// How to use:
    /// \code{.cpp}
    ///     Vector<char*> argv = engineBackend->GetCommandLineAsArgv();
    ///     ::UIApplicationMain(static_cast<int>(argv.size()), &*argv.begin(), @"principal", @"delegate");
    /// \endcode
    ///
    /// \return DAVA::Vector of char pointers to command line arguments.
    Vector<char*> GetCommandLineAsArgv();

    Engine* GetEngine() const;
    MainDispatcher* GetDispatcher() const;
    NativeService* GetNativeService() const;
    PlatformCore* GetPlatformCore() const;

    void SetOptions(KeyedArchive* options_);
    KeyedArchive* GetOptions();

    Window* InitializePrimaryWindow();

    void Init(eEngineRunMode engineRunMode, const Vector<String>& modules);
    int Run();
    void Quit(int32 exitCode);

    void SetCloseRequestHandler(const Function<bool(Window*)>& handler);
    void DispatchOnMainThread(const Function<void()>& task, bool blocking);
    void PostAppTerminate(bool triggeredBySystem);

    void OnGameLoopStarted();
    void OnGameLoopStopped();
    void OnBeforeTerminate();

    int32 OnFrame();

    void InitRenderer(Window* w);
    void ResetRenderer(Window* w, bool resetToNull);
    void DeinitRender(Window* w);

private:
    void RunConsole();

    void DoEvents();

    void OnFrameConsole();

    void OnBeginFrame();
    void OnUpdate(float32 frameDelta);
    void OnDraw();
    void OnEndFrame();

    void EventHandler(const MainDispatcherEvent& e);
    void HandleWindowCreated(const MainDispatcherEvent& e);
    void HandleWindowDestroyed(const MainDispatcherEvent& e);
    void HandleAppSuspended(const MainDispatcherEvent& e);
    void HandleAppResumed(const MainDispatcherEvent& e);
    void HandleAppTerminate(const MainDispatcherEvent& e);
    void HandleUserCloseRequest(const MainDispatcherEvent& e);

    void CreateSubsystems(const Vector<String>& modules);
    void DestroySubsystems();

private:
    // TODO: replace raw pointers with std::unique_ptr after work is done
    MainDispatcher* dispatcher = nullptr;
    PlatformCore* platformCore = nullptr;
    EngineContext* context = nullptr;
    Vector<String> cmdargs;

    Engine* engine = nullptr;

    Window* primaryWindow = nullptr;
    Set<Window*> justCreatedWindows; // Just created Window instances which do not have native windows yet
    Set<Window*> aliveWindows; // Windows which have native windows and take part in update cycle
    Set<Window*> dyingWindows; // Windows which will be deleted soon; native window may be already destroyed

    // Applciation-supplied functor which is invoked when user is trying to close window or application
    Function<bool(Window*)> closeRequestHandler;

    eEngineRunMode runMode = eEngineRunMode::GUI_STANDALONE;
    bool quitConsole = false;
    bool appIsSuspended = false;
    bool appIsTerminating = false;

    int32 exitCode = 0;

    KeyedArchive* options = nullptr;
    uint32 globalFrameIndex = 0;

    static EngineBackend* instance;
};

inline eEngineRunMode EngineBackend::GetRunMode() const
{
    return runMode;
}

inline bool EngineBackend::IsStandaloneGUIMode() const
{
    return runMode == eEngineRunMode::GUI_STANDALONE;
}

inline bool EngineBackend::IsEmbeddedGUIMode() const
{
    return runMode == eEngineRunMode::GUI_EMBEDDED;
}

inline bool EngineBackend::IsConsoleMode() const
{
    return runMode == eEngineRunMode::CONSOLE_MODE;
}

inline EngineContext* EngineBackend::GetEngineContext() const
{
    return context;
}

inline Window* EngineBackend::GetPrimaryWindow() const
{
    return primaryWindow;
}

inline uint32 EngineBackend::GetGlobalFrameIndex() const
{
    return globalFrameIndex;
}

inline int32 EngineBackend::GetExitCode() const
{
    return exitCode;
}

inline const Vector<String>& EngineBackend::GetCommandLine() const
{
    return cmdargs;
}

inline Engine* EngineBackend::GetEngine() const
{
    return engine;
}

inline MainDispatcher* EngineBackend::GetDispatcher() const
{
    return dispatcher;
}

inline PlatformCore* EngineBackend::GetPlatformCore() const
{
    return platformCore;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
