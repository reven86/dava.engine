#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_COREV2__)

#include "Base/RefPtr.h"
#include "Functional/Functional.h"

#include "Engine/EngineTypes.h"
#include "Engine/Private/EnginePrivateFwd.h"

#include "UI/UIEvent.h"

#include "Render/RHI/rhi_Type.h"

namespace DAVA
{
class KeyedArchive;
namespace Private
{
class EngineBackend final
{
public:
    static EngineBackend* Instance();
    static bool showingModalMessageBox;

    static WindowBackend* GetWindowBackend(Window* w);

    EngineBackend(const Vector<String>& cmdargs);
    ~EngineBackend();

    EngineBackend(const EngineBackend&) = delete;
    EngineBackend& operator=(const EngineBackend&) = delete;

    void EngineCreated(Engine* engine_);
    void EngineDestroyed();

    //////////////////////////////////////////////////////////////////////////
    eEngineRunMode GetRunMode() const;
    bool IsStandaloneGUIMode() const;
    bool IsEmbeddedGUIMode() const;
    bool IsConsoleMode() const;

    const EngineContext* GetContext() const;
    Window* GetPrimaryWindow() const;
    const Vector<Window*>& GetWindows() const;
    uint32 GetGlobalFrameIndex() const;
    int32 GetExitCode() const;
    const Vector<String>& GetCommandLine() const;
    Vector<char*> GetCommandLineAsArgv();

    Engine* GetEngine() const;
    MainDispatcher* GetDispatcher() const;
    PlatformCore* GetPlatformCore() const;

    const KeyedArchive* GetOptions() const;

    bool IsSuspended() const;

    Window* InitializePrimaryWindow();

    void Init(eEngineRunMode engineRunMode, const Vector<String>& modules, KeyedArchive* options_);
    int Run();
    void Quit(int32 exitCode_);

    void SetCloseRequestHandler(const Function<bool(Window*)>& handler);
    void DispatchOnMainThread(const Function<void()>& task, bool blocking);
    void PostAppTerminate(bool triggeredBySystem);
    void PostUserCloseRequest();

    void OnGameLoopStarted();
    void OnGameLoopStopped();
    void OnEngineCleanup();

    void OnWindowCreated(Window* window);
    void OnWindowDestroyed(Window* window);

    int32 OnFrame();

    void InitRenderer(Window* w);
    void ResetRenderer(Window* w, bool resetToNull);
    void DeinitRender(Window* w);

    void UpdateDisplayConfig();

    // Proxy method that calls SystemTimer::Adjust to prevent many friends to SystemTimer
    static void AdjustSystemTimer(int64 adjustMicro);

    void SetScreenTimeoutEnabled(bool enabled);
    bool IsScreenTimeoutEnabled() const;

    bool IsRunning() const;

    // This method sets the flag that indicates to draw a single frame while app is suspended (the flag is checked in the main loop)
    // It's used only on Android for now, since we do not resume renderer until onResume is called,
    // but it leads to a black screen if we have another non fullscreen activity on top and surface was destroyed while it's active
    // This eliminates black screen and shows a correct image instead
    void DrawSingleFrameWhileSuspended();

private:
    void RunConsole();

    void DoEvents();

    void OnFrameConsole();

    void BeginFrame();
    void Update(float32 frameDelta);
    void UpdateAndDrawWindows(float32 frameDelta, bool drawOnly);
    void EndFrame();
    void BackgroundUpdate(float32 frameDelta);

    void EventHandler(const MainDispatcherEvent& e);
    void HandleAppSuspended(const MainDispatcherEvent& e);
    void HandleAppResumed(const MainDispatcherEvent& e);
    void HandleBackNavigation(const MainDispatcherEvent& e);
    void HandleAppTerminate(const MainDispatcherEvent& e);
    void HandleUserCloseRequest(const MainDispatcherEvent& e);

    void CreateSubsystems(const Vector<String>& modules);
    void DestroySubsystems();

    void OnWindowVisibilityChanged(Window* window, bool visible);

    // These two methods are used instead of rhi::SuspendRendering and rhi::ResumeRendering
    // They check if we've already suspended or resumed the renderer and do nothing if we already have
    void SuspendRenderer();
    void ResumeRenderer();

    static void OnRenderingError(rhi::RenderingError err, void* param);

    // TODO: replace raw pointers with std::unique_ptr after work is done
    MainDispatcher* dispatcher = nullptr;
    PlatformCore* platformCore = nullptr;
    EngineContext* context = nullptr;
    Vector<String> cmdargs;

    Engine* engine = nullptr;

    Window* primaryWindow = nullptr;
    Set<Window*> justCreatedWindows; // Just created Window instances which do not have native windows yet
    Vector<Window*> aliveWindows; // Windows which have native windows and take part in update cycle
    Set<Window*> dyingWindows; // Windows which will be deleted soon; native window may be already destroyed

    // Application-supplied functor which is invoked when user is trying to close window or application
    Function<bool(Window*)> closeRequestHandler;

    eEngineRunMode runMode = eEngineRunMode::GUI_STANDALONE;
    bool isInitialized = false; // Flag indicating that Init method has been called
    bool quitConsole = false;
    bool appIsSuspended = false;
    bool appIsTerminating = false;

    int32 exitCode = 0;

    RefPtr<KeyedArchive> options;
    uint32 globalFrameIndex = 1;

    bool isRunning = false;

    bool atLeastOneWindowIsVisible = false;
    bool screenTimeoutEnabled = true;

    bool rendererSuspended = false;
    bool drawSingleFrameWhileSuspended = false;

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

inline const EngineContext* EngineBackend::GetContext() const
{
    return context;
}

inline Window* EngineBackend::GetPrimaryWindow() const
{
    return primaryWindow;
}

inline const Vector<Window*>& EngineBackend::GetWindows() const
{
    return aliveWindows;
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

inline bool EngineBackend::IsScreenTimeoutEnabled() const
{
    return screenTimeoutEnabled;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
