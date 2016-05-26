#include "Base/Platform.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Utils/Utils.h"

#include "Platform/TemplateWin32/CorePlatformWinUAP.h"
#include "Platform/TemplateWin32/WinUAPXamlApp.h"
#include "Platform/TemplateWin32/DispatcherWinUAP.h"

using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::ViewManagement;

namespace DAVA
{
int Core::Run(int /*argc*/, char* /*argv*/ [], AppHandle /*handle*/)
{
    std::unique_ptr<CorePlatformWinUAP> core = std::make_unique<CorePlatformWinUAP>();
    core->InitArgs();
    core->Run();
    return 0;
}

//////////////////////////////////////////////////////////////////////////
void CorePlatformWinUAP::InitArgs()
{
    SetCommandLine(WStringToString(::GetCommandLineW()));
}

void CorePlatformWinUAP::Run()
{
    auto appStartCallback = ref new ApplicationInitializationCallback([this](ApplicationInitializationCallbackParams ^ ) {
        xamlApp = ref new WinUAPXamlApp();
    });
    Application::Start(appStartCallback);
}

void CorePlatformWinUAP::Quit()
{
    xamlApp->SetQuitFlag();
}

Core::eScreenMode CorePlatformWinUAP::GetScreenMode()
{
    // will be called from UI thread
    ApplicationViewWindowingMode viewMode;
    auto func = [this, &viewMode] { viewMode = xamlApp->GetScreenMode(); };
    RunOnUIThreadBlocked(func);
    switch (viewMode)
    {
    case ApplicationViewWindowingMode::FullScreen:
        return eScreenMode::FULLSCREEN;
    case ApplicationViewWindowingMode::PreferredLaunchViewSize:
        return eScreenMode::WINDOWED;
    case ApplicationViewWindowingMode::Auto:
    default:
        // Unknown screen mode -> return default value
        return eScreenMode::FULLSCREEN;
    }
}

bool CorePlatformWinUAP::SetScreenMode(eScreenMode screenMode)
{
    switch (screenMode)
    {
    case DAVA::Core::eScreenMode::FULLSCREEN:
        RunOnUIThread([this]() { xamlApp->SetScreenMode(ApplicationViewWindowingMode::FullScreen); });
        return true;
    case DAVA::Core::eScreenMode::WINDOWED_FULLSCREEN:
        Logger::Error("Unimplemented screen mode");
        return false;
    case DAVA::Core::eScreenMode::WINDOWED:
        RunOnUIThread([this]() { xamlApp->SetScreenMode(ApplicationViewWindowingMode::PreferredLaunchViewSize); });
        return true;
    default:
        DVASSERT_MSG(false, "Unknown screen mode");
        return false;
    }
}

void CorePlatformWinUAP::SetWindowMinimumSize(float32 width, float32 height)
{
    xamlApp->SetWindowMinimumSize(width, height);
}

Vector2 CorePlatformWinUAP::GetWindowMinimumSize() const
{
    return xamlApp->GetWindowMinimumSize();
}

bool CorePlatformWinUAP::IsUIThread() const
{
    return xamlApp->UIThreadDispatcher()->HasThreadAccess;
}

void CorePlatformWinUAP::RunOnUIThread(std::function<void()>&& fn, bool blocked)
{
    if (!blocked)
    {
        xamlApp->UIThreadDispatcher()->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler(std::forward<std::function<void()>>(fn)));
    }
    else
    {
        DispatcherWinUAP::BlockingTaskWrapper wrapper = xamlApp->MainThreadDispatcher()->GetBlockingTaskWrapper(std::forward<std::function<void()>>(fn));
        RunOnUIThread([&wrapper]() { wrapper.RunTask(); });
        wrapper.WaitTaskComplete();
    }
}

void CorePlatformWinUAP::RunOnMainThread(std::function<void()>&& fn, bool blocked)
{
    if (!blocked)
    {
        xamlApp->MainThreadDispatcher()->RunAsync(std::forward<std::function<void()>>(fn));
    }
    else
    {
        xamlApp->MainThreadDispatcher()->RunAsyncAndWait(std::forward<std::function<void()>>(fn));
    }
}

} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
