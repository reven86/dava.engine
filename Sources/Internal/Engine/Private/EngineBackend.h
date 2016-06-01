#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"
#include "Functional/Functional.h"

#include "Engine/Public/Window.h"
#include "Engine/Private/EngineFwd.h"

#include "UI/UIEvent.h"

namespace DAVA
{
class AppContext;
class Engine;

namespace Private
{
class EngineBackend final
{
public:
    EngineBackend(int argc, char* argv[]);
    ~EngineBackend();

    EngineBackend(const EngineBackend&) = delete;
    EngineBackend& operator=(const EngineBackend&) = delete;

    const Vector<String>& GetCommandLine() const;

    void Init(bool consoleMode_, const Vector<String>& modules);
    int Run();
    void Quit(int exitCode_);

    void OnGameLoopStarted();
    void OnGameLoopStopped();

    void DoEvents();
    int32 OnFrame();

    void OnFrameConsole();

    void OnBeginFrame();
    void OnDraw();
    void OnEndFrame();

    Window* CreateWindowFrontend(bool primary);
    void EventHandler(const DispatcherEvent& e);

    void HandleWindowCreated(const DispatcherEvent& e);
    void HandleWindowDestroyed(const DispatcherEvent& e);
    void HandleWindowSizeChanged(const DispatcherEvent& e);
    void HandleWindowFocusChanged(const DispatcherEvent& e);
    void HandleWindowVisibilityChanged(const DispatcherEvent& e);
    void HandleMouseClick(const DispatcherEvent& e);
    void HandleMouseWheel(const DispatcherEvent& e);
    void HandleMouseMove(const DispatcherEvent& e);
    void HandleKeyPress(const DispatcherEvent& e);
    void HandleKeyChar(const DispatcherEvent& e);

    void ClearMouseButtons();

    void RunAsyncOnMainThread(const Function<void()>& task);

    void CreateRenderer();
    void ResetRenderer();

    // TODO: replace raw pointers with std::unique_ptr after work is done
    Dispatcher* dispatcher = nullptr;
    PlatformCore* platformCore = nullptr;
    AppContext* context = nullptr;

    Vector<String> cmdargs;
    bool consoleMode = false;
    Engine* engine = nullptr;
    Window* primaryWindow = nullptr;
    KeyedArchive* options = nullptr;
    uint32 globalFrameIndex = 0;

    int exitCode = 0;

    Bitset<static_cast<size_t>(UIEvent::MouseButton::NUM_BUTTONS)> mouseButtonState;

    static EngineBackend* instance;
};

inline const Vector<String>& EngineBackend::GetCommandLine() const
{
    return cmdargs;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
