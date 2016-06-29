#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"
#include "Functional/Functional.h"

#include "Engine/Public/Window.h"
#include "Engine/Private/EngineFwd.h"

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
    AppContext* GetEngineContext() const;
    WindowBackend* GetPrimaryWindow() const;
    uint32 GetGlobalFrameIndex() const;
    int32 GetExitCode() const;
    const Vector<String>& GetCommandLine() const;

    Dispatcher* GetDispatcher() const;
    PlatformCore* GetPlatformCore() const;

    void SetOptions(KeyedArchive* options_);
    KeyedArchive* GetOptions();

    void Init(bool consoleMode_, const Vector<String>& modules);
    int Run();
    void Quit(int32 exitCode_);

    void RunAsyncOnMainThread(const Function<void()>& task);
    void PostAppTerminate();

    void OnGameLoopStarted();
    void OnGameLoopStopped();
    void OnBeforeTerminate();

    int32 OnFrame();

    void InitRenderer(WindowBackend* w);
    void ResetRenderer(WindowBackend* w, bool resetToNull);
    void DeinitRender(WindowBackend* w);

private:
    void RunConsole();

    void DoEvents();

    void OnFrameConsole();

    void OnBeginFrame();
    void OnUpdate(float32 frameDelta);
    void OnDraw();
    void OnEndFrame();

    void EventHandler(const DispatcherEvent& e);
    void HandleWindowCreated(const DispatcherEvent& e);
    void HandleWindowDestroyed(const DispatcherEvent& e);
    void HandleAppTerminate(const DispatcherEvent& e);

    WindowBackend* CreatePrimaryWindowBackend();

private:
    // TODO: replace raw pointers with std::unique_ptr after work is done
    Dispatcher* dispatcher = nullptr;
    PlatformCore* platformCore = nullptr;
    AppContext* context = nullptr;
    Vector<String> cmdargs;

    Engine* engine = nullptr;

    WindowBackend* primaryWindow = nullptr;
    Set<WindowBackend*> windows;

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

inline AppContext* EngineBackend::GetEngineContext() const
{
    return context;
}

inline WindowBackend* EngineBackend::GetPrimaryWindow() const
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

inline Dispatcher* EngineBackend::GetDispatcher() const
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
