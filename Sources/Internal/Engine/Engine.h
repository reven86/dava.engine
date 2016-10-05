#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"
#include "Functional/Functional.h"

#include "Engine/EngineTypes.h"
#include "Engine/EngineContext.h"
#include "Engine/Private/EnginePrivateFwd.h"

namespace DAVA
{
class KeyedArchive;

/// \ingroup engine
/// \brief Core component of dava.engine which manages application's control flow.
///
/// Client applications and other parts of dava.engine interact with Engine in one way or another.
/// For any application there is precisely one Engine object.
///
/// Engine's responsibilities are:
///     - It initializes low level platform dependent objects and works directly with operating system.
///     - It receives events from underlying system and dispatches them to client application through signals.
///     - It initializes dava.engine's modules or subsystems and provides them to application through EngineContext object.
///     - It manages window creation and destruction.
///     - It controls game loop.
///     - It performs some other important tasks:
///         - application exit,
///         - getting command line
///         - running arbitrary client-supplied functor in context of thread where game loop is running, etc.
///
/// The Engine object is intended to be the only singleton in dava.engine and is accessible through Instance() static method.
///
/// The Engine object <b>must</b> be created and initialized before any use of dava.engine's modules or subsystems. Only few subsystems
/// can be used immediately after Engine creation and before initialization:
///     1. Logger
///     2. FileSystem
///
/// C/C++ main() function is buried deep inside dava.engine and client application shall provide its entry function DAVAMain:
/// \code
/// int DAVAMain(DAVA::Vector<DAVA::String> cmdline);
/// \endcode
///
/// Engine can run game loop in several modes (see eEngineRunMode):
///     - as GUI application
///     - as console application
///     - as GUI application embedded into other framework, now only Qt is supported
///
/// The following sample shows how to use Engine:
/// \code
/// #include <Engine/Engine.h>
/// using namespace DAVA;
/// int DAVAMain(Vector<String> cmdline)
/// {
///     Engine engine; // Create Engine object
///     eEngineRunMode runmode = eEngineRunMode::CONSOLE_MODE;
///     if (cmdline.size() > 1)
///     {
///         // Depending on command line choose run mode
///         runmode = cmdline[1] == "--console" ? eEngineRunMode::CONSOLE_MODE : runmode;
///         runmode = cmdline[1] == "--gui" ? eEngineRunMode::GUI_STANDALONE : runmode;
///         // To run in embedded mode application must link with Qt
///         runmode = cmdline[1] == "--embedded" ? eEngineRunMode::GUI_EMBEDDED : runmode;
///     }
///     engine.Init(runmode); // Initialize engine
///     return engine.Run(); // Run game loop
/// }
/// \endcode
class Engine final
{
public:
    /// Returns a pointer to Engine object or null if Engine is not created yet.
    static Engine* Instance();

    Engine();
    ~Engine();

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    EngineContext* GetContext() const;
    NativeService* GetNativeService() const;
    Window* PrimaryWindow() const;

    eEngineRunMode GetRunMode() const;
    bool IsStandaloneGUIMode() const;
    bool IsEmbeddedGUIMode() const;
    bool IsConsoleMode() const;

    /// Initialize Engine object and create primary Window object (if not console mode).
    /// This method sets desired run mode, lists modules which should be created and assigns engine options.
    /// Engine object must be initialized before using dava.engine.
    ///
    /// Engine can run game loop in several modes (see eEngineRunMode):
    ///     - as GUI application (eEngineRunMode::GUI_STANDALONE)
    ///     - as console application (eEngineRunMode::CONSOLE_MODE)
    ///     - as GUI application embedded into other framework (eEngineRunMode::GUI_EMBEDDED)
    ///
    /// Optionally application may list dava.engine's modules (subsystems) which she wants to use during execution.
    /// For now application may choose to create only several subsystems:
    ///     - DownloadManager
    ///     - JobManager
    ///     - LocalizationSystem
    ///     - NetCore
    ///     - SoundSystem
    ///     - PackManager
    ///
    /// Other modules are always created implicitly depending on specified run mode, e.g. UIScreenManager, InputSystem are not
    /// created in console mode.
    ///
    /// Additionally application can optionally set options to tune dava.engine and its modules. If no options are given then dava.engine
    /// chooses some reasonable default value. List of options supported by dava.engine:
    /// | Option                          | Description | Default        |
    /// | ------------------------------- | ----------- | -------------- |
    /// | renderer                        |             | rhi::RHI_GLES2 |
    /// | rhi_threaded_frame_count        |             | 0              |
    /// | max_index_buffer_count          |             | 0              |
    /// | max_vertex_buffer_count         |             | 0              |
    /// | max_const_buffer_count          |             | 0              |
    /// | max_texture_count               |             | 0              |
    /// | max_texture_set_count           |             | 0              |
    /// | max_sampler_state_count         |             | 0              |
    /// | max_pipeline_state_count        |             | 0              |
    /// | max_depthstencil_state_count    |             | 0              |
    /// | max_render_pass_count           |             | 0              |
    /// | max_command_buffer_count        |             | 0              |
    /// | max_packet_list_count           |             | 0              |
    /// | shader_const_buffer_size        |             | 0              |
    ///
    /// Other options can be found in description for corresponding module.
    void Init(eEngineRunMode runMode, const Vector<String>& modules = Vector<String>(), KeyedArchive* options = nullptr);

    /// Run game loop, here application life begins.
    /// After entering Run() application starts receiving life-cycle signals.
    /// Some platforms (iOS, macOS) may not return from Run().
    /// Returns exit code value passed to Quit() or 0 by default.
    int Run();

    /// Quit application with given exit code.
    void Quit(int exitCode = 0);

    /// \brief Set handler which is invoked when user is trying to close window or application.
    ///
    /// Handler can prevent window/application closing by returning false. This ability is
    /// supported only on desktop platforms: win32 and macOS.
    /// Typical usage is to return false in handler to prevent immediate window/app closing
    /// and show dialog asking user whether she wants to close window/app. If she chooses to
    /// close window/app then application should call Window::Close or Engine::Quit.
    /// Handler is only invoked if window/app is closing by user request: by pressing Alt+F4 or
    /// by clicking mouse on window close button or by pressing Cmd+Q on macOS.
    ///
    /// \param handler Function object which takes Window* as parameter and returns bool.
    ///                If pointer to Window is null then user is trying to close whole application,
    ///                otherwise user is trying to close specified window.
    void SetCloseRequestHandler(const Function<bool(Window*)>& handler);
    void RunAsyncOnMainThread(const Function<void()>& task);
    void RunAndWaitOnMainThread(const Function<void()>& task);

    const KeyedArchive* GetOptions() const;

    uint32 GetGlobalFrameIndex() const;
    const Vector<String>& GetCommandLine() const;
    DAVA::Vector<char*> GetCommandLineAsArgv() const;

public:
    // Signals
    /// Emited just before entring game loop. Note: native windows are not created yet and
    /// renderer is not initialized
    Signal<> gameLoopStarted;
    /// Emited after exiting game loop, application should terminate.
    Signal<> gameLoopStopped;
    /// Last signal emited by Engine, after this signal dava.engine is dead.
    Signal<> cleanup;
    /// Emited when window is created and renderer is initialized.
    Signal<Window*> windowCreated;
    /// Emited when window is going to destroy.
    Signal<Window*> windowDestroyed;
    Signal<> beginFrame;
    /// Emited on each frame. Note: rendering should be performed on Window::update signal.
    Signal<float32> update;
    Signal<> draw;
    Signal<> endFrame;
    /// Emited when application is entered suspended state. This signal is fired only on platforms
    /// that supports suspending: UWP, iOS, Android. Rendering is stopped but update signal is emited if system permits.
    Signal<> suspended;
    /// Emited when application exits suspended state
    Signal<> resumed;

private:
    Private::EngineBackend* engineBackend = nullptr;

    // Friends
    friend class Private::EngineBackend;
};

} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
