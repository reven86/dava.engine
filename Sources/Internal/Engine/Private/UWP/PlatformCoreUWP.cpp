#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/UWP/PlatformCoreUWP.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/Window.h"
#include "Engine/UWP/NativeServiceUWP.h"
#include "Engine/UWP/XamlApplicationListener.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Dispatcher/MainDispatcherEvent.h"
#include "Engine/Private/UWP/Window/WindowBackendUWP.h"

#include "Platform/SystemTimer.h"
#include "Concurrency/Thread.h"
#include "Logger/Logger.h"
#include "Utils/Utils.h"
#include "Platform/DeviceInfo.h"

extern int DAVAMain(DAVA::Vector<DAVA::String> cmdline);

namespace DAVA
{
namespace Private
{
PlatformCore::PlatformCore(EngineBackend* engineBackend_)
    : engineBackend(engineBackend_)
    , dispatcher(engineBackend->GetDispatcher())
    , nativeService(new NativeService(this))
{
}

PlatformCore::~PlatformCore() = default;

void PlatformCore::Init()
{
}

void PlatformCore::Run()
{
    if (savedLaunchArgs != nullptr)
    {
        // Here notify listeners about OnLaunched
        engineBackend->GetPrimaryWindow()->RunAsyncOnUIThread([ this, savedLaunchArgs = savedLaunchArgs ]() {
            NotifyListeners(ON_LAUNCHED, savedLaunchArgs);
        });
        savedLaunchArgs = nullptr;
    }

    engineBackend->OnGameLoopStarted();

    while (!quitGameThread)
    {
        uint64 frameBeginTime = SystemTimer::Instance()->AbsoluteMS();

        int32 fps = engineBackend->OnFrame();

        uint64 frameEndTime = SystemTimer::Instance()->AbsoluteMS();
        uint32 frameDuration = static_cast<uint32>(frameEndTime - frameBeginTime);

        int32 sleep = 1;
        if (fps > 0)
        {
            sleep = 1000 / fps - frameDuration;
            if (sleep < 1)
                sleep = 1;
        }
        Sleep(sleep);
    }

    engineBackend->OnGameLoopStopped();
    engineBackend->OnEngineCleanup();
}

void PlatformCore::PrepareToQuit()
{
    engineBackend->PostAppTerminate(true);
}

void PlatformCore::Quit()
{
    quitGameThread = true;
}

void PlatformCore::OnLaunched(::Windows::ApplicationModel::Activation::LaunchActivatedEventArgs ^ launchArgs)
{
    if (!gameThreadRunning)
    {
        Thread* gameThread = Thread::Create(MakeFunction(this, &PlatformCore::GameThread));
        gameThread->Start();
        gameThread->BindToProcessor(0);
        // TODO: make Thread detachable
        //gameThread->Detach();
        //gameThread->Release();

        gameThreadRunning = true;

        // Save launch arguments if game thread is not runnnig yet and notify listeners later
        // when dava.engine is initialized and listeners have had chance to register.
        savedLaunchArgs = launchArgs;
    }
    else
    {
        NotifyListeners(ON_LAUNCHED, launchArgs);
    }
    if (launchArgs->Kind == Windows::ApplicationModel::Activation::ActivationKind::Launch)
    {
        Platform::String ^ launchString = launchArgs->Arguments;
        if (!launchString->IsEmpty())
        {
            String uidStr = UTF8Utils::EncodeToUTF8(launchString->Data());
            dispatcher.PostEvent(MainDispatcherEvent::CreateLocalNotificationEvent(uidStr));
        }
    }
}

void PlatformCore::OnActivated()
{
}

void PlatformCore::OnWindowCreated(::Windows::UI::Xaml::Window ^ xamlWindow)
{
    // TODO: think about binding XAML window to prior created Window instance
    Window* primaryWindow = engineBackend->GetPrimaryWindow();
    if (primaryWindow == nullptr)
    {
        primaryWindow = engineBackend->InitializePrimaryWindow();
    }
    WindowBackend* windowBackend = primaryWindow->GetBackend();
    windowBackend->BindXamlWindow(xamlWindow);
}

void PlatformCore::OnSuspending()
{
    dispatcher->SendEvent(MainDispatcherEvent(MainDispatcherEvent::APP_SUSPENDED)); // Blocking call !!!
}

void PlatformCore::OnResuming()
{
    dispatcher->PostEvent(MainDispatcherEvent(MainDispatcherEvent::APP_RESUMED));
}

void PlatformCore::OnUnhandledException(::Windows::UI::Xaml::UnhandledExceptionEventArgs ^ arg)
{
    Logger::Error("Unhandled exception: hresult=0x%08X, message=%s", arg->Exception, WStringToString(arg->Message->Data()).c_str());
}

void PlatformCore::OnBackPressed()
{
    dispatcher->PostEvent(MainDispatcherEvent(MainDispatcherEvent::BACK_NAVIGATION));
}

void PlatformCore::OnGamepadAdded(::Windows::Gaming::Input::Gamepad ^ /*gamepad*/)
{
    dispatcher->PostEvent(MainDispatcherEvent::CreateGamepadAddedEvent(0));
}

void PlatformCore::OnGamepadRemoved(::Windows::Gaming::Input::Gamepad ^ /*gamepad*/)
{
    dispatcher->PostEvent(MainDispatcherEvent::CreateGamepadRemovedEvent(0));
}

void PlatformCore::RegisterXamlApplicationListener(XamlApplicationListener* listener)
{
    DVASSERT(listener != nullptr);
    auto it = std::find(begin(xamlApplicationListeners), end(xamlApplicationListeners), listener);
    if (it == end(xamlApplicationListeners))
    {
        xamlApplicationListeners.push_back(listener);
    }
}

void PlatformCore::UnregisterXamlApplicationListener(XamlApplicationListener* listener)
{
    auto it = std::find(begin(xamlApplicationListeners), end(xamlApplicationListeners), listener);
    if (it != end(xamlApplicationListeners))
    {
        xamlApplicationListeners.erase(it);
    }
}

void PlatformCore::GameThread()
{
    Vector<String> cmdline = engineBackend->GetCommandLine();
    DAVAMain(std::move(cmdline));

    using namespace ::Windows::UI::Xaml;
    Application::Current->Exit();
}

void PlatformCore::NotifyListeners(eNotificationType type, ::Platform::Object ^ arg1)
{
    using ::Windows::ApplicationModel::Activation::LaunchActivatedEventArgs;
    using ::Windows::ApplicationModel::Activation::IActivatedEventArgs;

    for (auto i = begin(xamlApplicationListeners), e = end(xamlApplicationListeners); i != e;)
    {
        XamlApplicationListener* l = *i;
        ++i;
        switch (type)
        {
        case ON_LAUNCHED:
            l->OnLaunched(static_cast<LaunchActivatedEventArgs ^>(arg1));
            break;
        default:
            break;
        }
    }
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__
