#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"
#include "Functional/Functional.h"

#include "Engine/Public/Window.h"
#include "Engine/Private/EngineFwd.h"

#include "UI/UIEvent.h"

namespace DAVA
{
struct IGame;
class Engine;

namespace Private
{
class EngineBackend final
{
public:
    EngineBackend();
    ~EngineBackend();

    EngineBackend(const EngineBackend&) = delete;
    EngineBackend& operator=(const EngineBackend&) = delete;

    void Init(bool consoleMode_);
    int Run();
    void Quit();

    void OnGameLoopStarted();
    void OnGameLoopStopped();

    void DoEvents();
    void OnFrame();

    void OnBeginFrame();
    void OnDraw();
    void OnEndFrame();

    Window* CreateWindowFrontend(bool primary);
    void EventHandler(const DispatcherEvent& e);

    void HandleWindowCreated(const DispatcherEvent& e);
    void HandleWindowDestroyed(const DispatcherEvent& e);
    void HandleWindowSizeChanged(const DispatcherEvent& e);
    void HandleWindowFocusChanged(const DispatcherEvent& e);
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

    bool consoleMode = false;
    Engine* engine = nullptr;
    Window* primaryWindow = nullptr;

    Bitset<static_cast<size_t>(UIEvent::MouseButton::NUM_BUTTONS)> mouseButtonState;

    static EngineBackend* instance;
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
