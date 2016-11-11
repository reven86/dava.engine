#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"
#include "Functional/Functional.h"

#include "Engine/EngineTypes.h"
#include "Engine/EngineContext.h"
#include "Engine/Private/EnginePrivateFwd.h"

namespace DAVA
{
/**
    \ingroup engine
    Utility function to get engine context.

    Behaviour is undefined when called before `Engine` instantiated or after 'Engine::cleanup' signal emited.
    Another but longer way to get context is to call `Engine::Instance()->GetContext()`.
*/
const EngineContext* GetEngineContext();

/**
    \ingroup engine
    Utility function to get primary window.

    Behaviour is undefined when called before `Engine` instantiated or after 'Engine::cleanup' signal has emited.
    Return `nullptr` if called before `Engine::Init` or if `Engine` has been initialized with `eEngineRunMode::CONSOLE_MODE` mode.
    Another but longer way to get primary window is to call `Engine::Instance()->PrimaryWindow()`.
*/
Window* GetPrimaryWindow();

class KeyedArchive;
/**
    \ingroup engine
    Core component of dava.engine which manages application's control flow.
    
    Client applications and other parts of dava.engine interact with Engine in one way or another.
    For any application there is precisely one Engine object.
    
    Engine's responsibilities are:
        - It initializes low level platform dependent objects and works directly with operating system.
        - It receives events from underlying system and dispatches them to client application through signals.
        - It initializes dava.engine's modules or subsystems and provides them to application through EngineContext object.
        - It manages window creation and destruction.
        - It controls game loop.
        - It performs some other important tasks:
            - application exit,
            - getting command line
            - running arbitrary client-supplied functor in context of thread where game loop is running, etc.
    
    The Engine object is intended to be the only singleton in dava.engine and is accessible through Instance() static method.
    
    The Engine object **must** be created and initialized before any use of dava.engine's modules or subsystems. Only few subsystems
    can be used immediately after Engine creation and before initialization:
        1. Logger
        2. FileSystem
    
    C/C++ main() function is buried deep inside dava.engine and client application shall provide its entry function DAVAMain:
    \code
    int DAVAMain(DAVA::Vector<DAVA::String> cmdline);
    \endcode
    
    Engine can run game loop in several modes (see eEngineRunMode):
        - as GUI application
        - as console application
        - as GUI application embedded into other framework, now only Qt is supported
    
    The following sample shows how to use Engine:
    \code
    #include <Engine/Engine.h>
    using namespace DAVA;
    int DAVAMain(Vector<String> cmdline)
    {
        Engine engine; // Create Engine object
        eEngineRunMode runmode = eEngineRunMode::CONSOLE_MODE;
        if (cmdline.size() > 1)
        {
            // Depending on command line choose run mode
            runmode = cmdline[1] == "--console" ? eEngineRunMode::CONSOLE_MODE : runmode;
            runmode = cmdline[1] == "--gui" ? eEngineRunMode::GUI_STANDALONE : runmode;
            // To run in embedded mode application must link with Qt
            runmode = cmdline[1] == "--embedded" ? eEngineRunMode::GUI_EMBEDDED : runmode;
        }
        engine.Init(runmode, Vector<String>(), nullptr); // Initialize engine
        return engine.Run(); // Run game loop
    }
    \endcode
*/
class Engine final
{
public:
    /**
        Return a pointer to Engine object or null if Engine is not created yet.
    */
    static Engine* Instance();

    Engine();
    ~Engine();

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    /**
        Get engine context which holds all DAVA subsystems.

        Subsystems become available after `Engine::Init` call, which subsystems are available depends on
        Engine's run mode (see `eEngineRunMode`), target platform (android, win32, etc).
        The following subsystems are available immediately after Engine class creation:
            - Logger
            - FileSystem
            - DeviceManager
    */
    const EngineContext* GetContext() const;
    NativeService* GetNativeService() const;

    /**
        Return primary window if any.

        Primary window is non null except in the following cases:
            - before `Engine::Init` call,
            - Engine was initialized with `eEngineRunMode::CONSOLE_MODE` mode,
            - after receiving signal `Engine::windowDestroyed` for primary window.
    */
    Window* PrimaryWindow() const;

    eEngineRunMode GetRunMode() const;
    bool IsStandaloneGUIMode() const;
    bool IsEmbeddedGUIMode() const;
    bool IsConsoleMode() const;

    /**
        Initialize Engine object and create primary Window object (if not console mode).
        This method sets desired run mode, lists modules which should be created and assigns engine options.
        Engine object must be initialized before using dava.engine.
    
        Engine can run game loop in several modes (see eEngineRunMode enum):
            - as GUI application (eEngineRunMode::GUI_STANDALONE)
            - as console application (eEngineRunMode::CONSOLE_MODE)
            - as GUI application embedded into other framework (eEngineRunMode::GUI_EMBEDDED)
    
        Application may list dava.engine's modules (subsystems) which she wants to use during execution. List may be empty.
        For now application may choose to create only several subsystems:
            - DownloadManager
            - JobManager
            - LocalizationSystem
            - NetCore
            - SoundSystem
            - PackManager
    
        Other modules are always created implicitly depending on specified run mode, e.g. UIScreenManager, InputSystem are not
        created in console mode.
    
        Additionally application can set options to tune dava.engine and its modules (can be null). If no options are given then dava.engine
        chooses some reasonable default value. List of options supported by dava.engine:

        | **Render options**              | Description                | Default        |
        | ------------------------------- | -------------------------- | -------------- |
        | renderer                        |                            | rhi::RHI_GLES2 |
        | rhi_threaded_frame_count        |                            | 0              |
        | max_index_buffer_count          |                            | 0              |
        | max_vertex_buffer_count         |                            | 0              |
        | max_const_buffer_count          |                            | 0              |
        | max_texture_count               |                            | 0              |
        | max_texture_set_count           |                            | 0              |
        | max_sampler_state_count         |                            | 0              |
        | max_pipeline_state_count        |                            | 0              |
        | max_depthstencil_state_count    |                            | 0              |
        | max_render_pass_count           |                            | 0              |
        | max_command_buffer_count        |                            | 0              |
        | max_packet_list_count           |                            | 0              |
        | shader_const_buffer_size        |                            | 0              |

        For more info on render options ask RHI guys.
    
        Other options can be found in description for corresponding module.
    */
    void Init(eEngineRunMode runMode, const Vector<String>& modules, KeyedArchive* options);

    /**
        Run game loop, here application life begins.
        After entering Run() application starts receiving life-cycle signals.
        Some platforms (iOS, macOS) may not return from Run(). Application should subscribe
        to cleanup signal as last chance to free up resources and get ready for termination.

        \return Exit code value passed to Quit() or 0 by default.
    */
    int Run();

    /**
        Quit application with given exit code.
    */
    void Quit(int exitCode = 0);

    /**
        Set handler which is invoked when user is trying to close window or application.
        
        Handler can prevent window/application closing by returning false. This ability is
        supported only on desktop platforms: win32 and macOS.
        Typical usage is to return false in handler to prevent immediate window/app closing
        and show dialog asking user whether she wants to close window/app. If she chooses to
        close window/app then application should call Window::Close or Engine::Quit.
        Handler is only invoked if window/app is closing by user request: by pressing Alt+F4 or
        by clicking mouse on window close button or by pressing Cmd+Q on macOS.
        
        \param handler Function object with prototype
                       \code
                            bool handler(Window* window);
                       \endcode
                       If window parameter is null then user is trying to close whole application,
                       otherwise user is trying to close specified window.
    */
    void SetCloseRequestHandler(const Function<bool(Window*)>& handler);

    /**
        Run given task in DAVA main thread context without waiting task execution.
        This method can be called from any thread.
    */
    void RunAsyncOnMainThread(const Function<void()>& task);

    /**
        Run given task in DAVA main thread context and block calling thread until task is executed.
        This method can be called from any thread.
    */
    void RunAndWaitOnMainThread(const Function<void()>& task);

    const KeyedArchive* GetOptions() const;

    /**
        Get current frame index (you can consider this index as number of updates invoked).
    */
    uint32 GetGlobalFrameIndex() const;

    /**
        Get parsed command line args, now it is the same as command line passed to DAVAMain function.
        First command line argument is always application name (on android it is app_process).
    */
    const Vector<String>& GetCommandLine() const;

    /**
        Get command line args in form suitable to use as argc and argv.

        Some platforms want command line in form argc and argv, e.g. iOS's UIApplicationMain, Qt's QApplication, etc.
        Engine grabs command line at program start and stores it as DAVA::Vector of DAVA::String, so GetCommandLineAsArgv allows
        passing command line arguments to those functions.

        How to use:
        \code{.cpp}
            Vector<char*> argv = engineBackend->GetCommandLineAsArgv();
            int n = static_cast<int>(argv.size());
            ::UIApplicationMain(n, &*argv.begin(), @"principal", @"delegate");
        \endcode
    */
    Vector<char*> GetCommandLineAsArgv() const;

public:
    Signal<> gameLoopStarted; //!< Emited just before entring game loop. Note: native windows are not created yet and renderer is not initialized.
    Signal<> gameLoopStopped; //!< Emited after exiting game loop, application should terminate.
    Signal<> cleanup; //!< Last signal emited by Engine, after this signal dava.engine is dead.
    Signal<Window*> windowCreated; //!< Emited when native window is created and renderer is initialized.
    Signal<Window*> windowDestroyed; //!< Emited when native window is destroyed. After this signal no one should use window.
    Signal<> beginFrame;
    Signal<float32> update; //!< Emited on each frame. Note: rendering should be performed on Window::update signal.
    Signal<> draw;
    Signal<> endFrame;
    Signal<> suspended; //!< Emited when application has entered suspended state. This signal is fired only on platforms
    //!< that support suspending: UWP, iOS, Android. Rendering is stopped but update signal is emited if system permits.
    Signal<> resumed; //!< Emited when application exits suspended state.

private:
    Private::EngineBackend* engineBackend = nullptr;

    // Friends
    friend class Private::EngineBackend;
};

} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
