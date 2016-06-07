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
    EngineBackend(int argc, char* argv[]);
    ~EngineBackend();

    EngineBackend(const EngineBackend&) = delete;
    EngineBackend& operator=(const EngineBackend&) = delete;

    //////////////////////////////////////////////////////////////////////////
    const Vector<String>& GetCommandLine() const;

    void Init(bool consoleMode_, const Vector<String>& modules);
    int Run();
    void Quit(int exitCode_);

    void RunAsyncOnMainThread(const Function<void()>& task);

    //////////////////////////////////////////////////////////////////////////

    void RunConsole();

    void OnGameLoopStarted();
    void OnGameLoopStopped();

    void DoEvents();
    int32 OnFrame();

    void OnFrameConsole();

    void OnBeginFrame();
    void OnUpdate(float32 frameDelta);
    void OnDraw();
    void OnEndFrame();

    void InitRenderer(WindowBackend* w);
    void ResetRenderer(WindowBackend* w, bool resetToNull);
    void DeinitRender(WindowBackend* w);

    void EventHandler(const DispatcherEvent& e);
    void HandleWindowCreated(const DispatcherEvent& e);
    void HandleWindowDestroyed(const DispatcherEvent& e);

    // TODO: replace raw pointers with std::unique_ptr after work is done
    Dispatcher* dispatcher = nullptr;
    PlatformCore* platformCore = nullptr;
    AppContext* context = nullptr;

    Vector<String> cmdargs;
    Engine* engine = nullptr;
    WindowBackend* primaryWindow = nullptr;
    Vector<WindowBackend*> windows;

    bool consoleMode = false;

    bool quitConsole = false;
    int exitCode = 0;

    KeyedArchive* options = nullptr;
    uint32 globalFrameIndex = 0;

    static EngineBackend* instance;
};

inline const Vector<String>& EngineBackend::GetCommandLine() const
{
    return cmdargs;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
