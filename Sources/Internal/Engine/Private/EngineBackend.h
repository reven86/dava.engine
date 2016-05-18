#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"
#include "Functional/Functional.h"

#include "Engine/Public/Window.h"
#include "Engine/Private/EngineFwd.h"

namespace DAVA
{
struct IGame;

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
    int Run(IGame* gameObject);
    void Quit();

    void OnGameLoopStarted();
    void OnGameLoopStopped();
    void DoEvents();
    void OnFrame();

    Window* CreateWindowFrontend(bool primary);
    void EventHandler(const DispatcherEvent& e);

    void RunAsyncOnMainThread(const Function<void()>& task);

    // TODO: replace raw pointers with std::unique_ptr after work is done
    Dispatcher* dispatcher = nullptr;
    PlatformCore* platformCore = nullptr;

    bool consoleMode = false;
    IGame* game = nullptr;
    Window* primaryWindow = nullptr;

    static EngineBackend* instance;
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
