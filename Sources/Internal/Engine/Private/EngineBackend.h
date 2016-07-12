#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"
#include "Functional/Functional.h"

#include "Engine/Public/Window.h"
#include "Engine/Private/EnginePrivateFwd.h"

#include "UI/UIEvent.h"

namespace DAVA
{
namespace Private
{
class EngineBackend final
{
public:
    static EngineBackend* Instance();

    EngineBackend(const Vector<String>& cmdargs_);
    ~EngineBackend();

    EngineBackend(const EngineBackend&) = delete;
    EngineBackend& operator=(const EngineBackend&) = delete;

    void EngineCreated(Engine* e);
    void EngineDestroyed();

    //////////////////////////////////////////////////////////////////////////
    bool IsConsoleMode() const;
    EngineContext* GetEngineContext() const;
    Window* GetPrimaryWindow() const;
    uint32 GetGlobalFrameIndex() const;
    int32 GetExitCode() const;
    const Vector<String>& GetCommandLine() const;

    Engine* GetEngine() const;
    MainDispatcher* GetDispatcher() const;
    NativeService* GetNativeService() const;
    PlatformCore* GetPlatformCore() const;

    void SetOptions(KeyedArchive* options_);
    KeyedArchive* GetOptions();

    void Init(bool consoleMode_, const Vector<String>& modules);
    int Run();
    void Quit(int32 exitCode_);

    void RunAsyncOnMainThread(const Function<void()>& task);
    void RunAndWaitOnMainThread(const Function<void()>& task);
    void PostAppTerminate();

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
    void HandleAppTerminate(const MainDispatcherEvent& e);

    Window* CreatePrimaryWindowBackend();

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
    Set<Window*> windows;

    bool consoleMode = false;
    bool quitConsole = false;
    bool appIsTerminating = false;

    int32 exitCode = 0;

    KeyedArchive* options = nullptr;
    uint32 globalFrameIndex = 0;

    static EngineBackend* instance;
};

inline bool EngineBackend::IsConsoleMode() const
{
    return consoleMode;
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
