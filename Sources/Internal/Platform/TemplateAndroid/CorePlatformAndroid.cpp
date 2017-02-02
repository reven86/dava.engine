#include "Base/Platform.h"


#if !defined(__DAVAENGINE_COREV2__)

#if defined(__DAVAENGINE_ANDROID__)

extern void FrameworkDidLaunched();
extern void FrameworkWillTerminate();

#include "Platform/DeviceInfo.h"
#include "Input/InputSystem.h"
#include "UI/UIEvent.h"
#include "FileSystem/FileSystem.h"
#include "Platform/TemplateAndroid/AssetsManagerAndroid.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Engine/Android/JNIBridge.h"
#include "Platform/TemplateAndroid/CorePlatformAndroid.h"
#include "Debug/DVAssertDefaultHandlers.h"

namespace DAVA
{
AndroidSystemDelegate::AndroidSystemDelegate(JavaVM* vm)
{
    Logger::Debug("AndroidSystemDelegate::AndroidSystemDelegate()");

    this->vm = vm;
    environment = NULL;
    if (vm->GetEnv(reinterpret_cast<void**>(&environment), JNI_VERSION_1_4) != JNI_OK)
    {
        Logger::Debug("Failed to get the environment using GetEnv()");
    }
}

Core::eDeviceFamily Core::GetDeviceFamily()
{
    float32 width = UIControlSystem::Instance()->vcs->GetPhysicalScreenSize().dx;
    float32 height = UIControlSystem::Instance()->vcs->GetPhysicalScreenSize().dy;
    float32 dpi = GetScreenDPI();

    float32 inches = sqrt((width * width) + (height * height)) / dpi;

    if (inches > 6.f)
        return DEVICE_PAD;

    return DEVICE_HANDSET;
}

CorePlatformAndroid::CorePlatformAndroid(const String& cmdLine)
{
    Assert::SetupDefaultHandlers();
    SetCommandLine(cmdLine);
}

int Core::Run(int argc, char* argv[], AppHandle handle)
{
    //      CoreWin32Platform * core = new CoreWin32Platform();
    //      core->CreateWin32Window(handle);
    //      core->Run();
    //      core->ReleaseSingletons();
    return 0;
}

void CorePlatformAndroid::Quit()
{
    Logger::Debug("[CorePlatformAndroid::Quit]");
    renderIsActive = false;
    // finish java activity
    JNI::JavaClass javaClass("com/dava/framework/JNIActivity");
    Function<void()> finishActivity = javaClass.GetStaticMethod<void>("finishActivity");
    finishActivity();
}

void CorePlatformAndroid::QuitAction()
{
    Logger::Debug("[CorePlatformAndroid::QuitAction] in");

    if (Core::Instance())
    {
        // will call gameCore->onAppFinished() destroy game singletons
        Core::Instance()->SystemAppFinished();
    }

    FrameworkWillTerminate();

    Logger::Debug("[CorePlatformAndroid::QuitAction] out");
}

void CorePlatformAndroid::ProcessFrame()
{
    if (renderIsActive)
    {
        if (viewSizeChanged)
        {
            ApplyPendingViewSize();
        }

        Core::SystemProcessFrame();
    }
}

void CorePlatformAndroid::SetScreenScaleMultiplier(float32 multiplier)
{
    float32 curMultiplier = Core::GetScreenScaleMultiplier();
    if (!FLOAT_EQUAL(multiplier, curMultiplier))
    {
        Core::SetScreenScaleMultiplier(multiplier);
        RenderReset(pendingWidth, pendingHeight);
    }
}

void CorePlatformAndroid::ApplyPendingViewSize()
{
    Logger::Debug("[CorePlatformAndroid::ApplyPendingViewSize] in");
    Logger::Debug("[CorePlatformAndroid::] w = %d, h = %d, surfW = %d, surfH = %d", pendingWidth, pendingHeight, backbufferWidth, backbufferHeight);

    viewSizeChanged = false;

    DeviceInfo::InitializeScreenInfo();

    UIControlSystem::Instance()->vcs->SetInputScreenAreaSize(pendingWidth, pendingHeight);
    UIControlSystem::Instance()->vcs->SetPhysicalScreenSize(backbufferWidth, backbufferHeight);
    UIControlSystem::Instance()->vcs->ScreenSizeChanged();

    Logger::Debug("[CorePlatformAndroid::ApplyPendingViewSize] out");
    Logger::FrameworkDebug("[CorePlatformAndroid::UpdateScreenMode] done");
}

void CorePlatformAndroid::CreateAndroidWindow(const char8* docPathEx, const char8* docPathIn, const char8* assets, const char8* logTag, AndroidSystemDelegate* sysDelegate)
{
    androidDelegate = sysDelegate;
    externalStorage = docPathEx;
    internalStorage = docPathIn;

    Core::CreateSingletons();

    new AssetsManagerAndroid(assets);

    Logger::SetTag(logTag);
}

void CorePlatformAndroid::RenderReset(int32 w, int32 h)
{
    Logger::Debug("[CorePlatformAndroid::RenderReset] start");

    renderIsActive = true;

    pendingWidth = w;
    pendingHeight = h;

    // Android is always using hardware scaler for rendering.
    // By specifying a scaled size for the buffer, the hardware
    // scaler will be enabled and we will benefit in our rendering
    // to the specified window (buffer is going to be up-scaled
    // or down-scaled to the window size).
    // For more info see: http://android-developers.blogspot.com.by/2013/09/using-hardware-scaler-for-performance.html
    float32 scale = Core::GetScreenScaleMultiplier();
    backbufferWidth = static_cast<int32>(scale * w);
    backbufferHeight = static_cast<int32>(scale * h);

    viewSizeChanged = true;

    if (wasCreated)
    {
        rhi::ResetParam params;
        params.width = static_cast<uint32>(backbufferWidth);
        params.height = static_cast<uint32>(backbufferHeight);
        params.window = rendererParams.window;
        Renderer::Reset(params);
    }
    else
    {
        wasCreated = true;

        ApplyPendingViewSize();
        rendererParams.width = static_cast<uint32>(backbufferWidth);
        rendererParams.height = static_cast<uint32>(backbufferHeight);

        // Set proper width and height before call FrameworkDidlaunched
        FrameworkDidLaunched();

        Core::Instance()->SystemAppStarted();

        // We are always in foreground when initialize application
        // This condition avoids render resuming on startup
        // Render resuming on startup has no sence, but can breaks render
        foreground = true;
        StartForeground();
    }

    Logger::Debug("[CorePlatformAndroid::RenderReset] end");
}

void CorePlatformAndroid::OnCreateActivity()
{
    DAVA::Thread::InitMainThread();
}

void CorePlatformAndroid::OnDestroyActivity()
{
    Logger::Info("[CorePlatformAndroid::OnDestroyActivity]");

    renderIsActive = false;
    wasCreated = false;

    QuitAction();
}

void CorePlatformAndroid::StartVisible()
{
    //      Logger::Debug("[CorePlatformAndroid::StartVisible]");
}

void CorePlatformAndroid::StopVisible()
{
    //      Logger::Debug("[CorePlatformAndroid::StopVisible]");
}

void CorePlatformAndroid::StartForeground()
{
    Logger::Debug("[CorePlatformAndroid::StartForeground] in");

    if (wasCreated)
    {
        DAVA::ApplicationCore* core = DAVA::Core::Instance()->GetApplicationCore();

        if (!core)
        {
            DAVA::Core::Instance()->SetIsActive(true);
        }

        DAVA::Core::Instance()->GoForeground();
        DAVA::Core::Instance()->FocusReceived();

        if (!foreground)
            rhi::ResumeRendering();

        if (core)
        {
            core->OnResume();
        }

        foreground = true;
    }
    Logger::Debug("[CorePlatformAndroid::StartForeground] out");
}

void CorePlatformAndroid::StopForeground(bool isLock)
{
    Logger::Debug("[CorePlatformAndroid::StopForeground] in");

    DAVA::ApplicationCore* core = DAVA::Core::Instance()->GetApplicationCore();
    if (core)
    {
        core->OnSuspend();
    }
    else
    {
        DAVA::Core::Instance()->SetIsActive(false);
    }
    DAVA::Core::Instance()->GoBackground(isLock);
    DAVA::Core::Instance()->FocusLost();

    if (foreground)
        rhi::SuspendRendering();

    foreground = false;

    Logger::Debug("[CorePlatformAndroid::StopForeground] out");
}

void CorePlatformAndroid::KeyUp(int32 keyCode, uint32 modifiers)
{
    InputSystem* inputSystem = InputSystem::Instance();
    KeyboardDevice& keyboard = inputSystem->GetKeyboard();

    UIEvent keyEvent;
    keyEvent.device = eInputDevices::KEYBOARD;
    keyEvent.phase = DAVA::UIEvent::Phase::KEY_UP;
    keyEvent.key = keyboard.GetDavaKeyForSystemKey(keyCode);
    keyEvent.timestamp = SystemTimer::GetMs() / 1000.0;
    keyEvent.modifiers = modifiers;

    inputSystem->ProcessInputEvent(&keyEvent);

    keyboard.OnKeyUnpressed(keyEvent.key);
}

void CorePlatformAndroid::KeyDown(int32 keyCode, uint32 modifiers)
{
    InputSystem* inputSystem = InputSystem::Instance();
    KeyboardDevice& keyboard = inputSystem->GetKeyboard();

    UIEvent keyEvent;
    keyEvent.device = eInputDevices::KEYBOARD;
    keyEvent.phase = DAVA::UIEvent::Phase::KEY_DOWN;
    keyEvent.key = keyboard.GetDavaKeyForSystemKey(keyCode);
    keyEvent.timestamp = SystemTimer::GetMs() / 1000.0;
    keyEvent.modifiers = modifiers;

    inputSystem->ProcessInputEvent(&keyEvent);

    keyboard.OnKeyPressed(keyEvent.key);
}

void CorePlatformAndroid::OnGamepadElement(int32 elementKey, float32 value, bool isKeycode, uint32 modifiers)
{
    GamepadDevice& gamepadDevice = InputSystem::Instance()->GetGamepadDevice();

    uint32 davaKey = GamepadDevice::INVALID_DAVAKEY;
    if (isKeycode)
    {
        davaKey = gamepadDevice.GetDavaEventIdForSystemKeycode(elementKey);
    }
    else
    {
        davaKey = gamepadDevice.GetDavaEventIdForSystemAxis(elementKey);
    }

    if (davaKey == GamepadDevice::INVALID_DAVAKEY)
    {
        Logger::Debug("unknown gamepad element code: 0x%H", elementKey);
        return;
    }

    UIEvent newEvent;
    newEvent.element = static_cast<GamepadDevice::eDavaGamepadElement>(davaKey);
    newEvent.physPoint.x = value;
    newEvent.point.x = value;
    newEvent.phase = DAVA::UIEvent::Phase::JOYSTICK;
    newEvent.device = eInputDevices::GAMEPAD;
    newEvent.timestamp = SystemTimer::GetMs() / 1000.0;
    newEvent.modifiers = modifiers;

    gamepadDevice.SystemProcessElement(static_cast<GamepadDevice::eDavaGamepadElement>(davaKey), value);
    InputSystem::Instance()->ProcessInputEvent(&newEvent);
}

void CorePlatformAndroid::OnGamepadAvailable(bool isAvailable)
{
    InputSystem::Instance()->GetGamepadDevice().SetAvailable(isAvailable);
}

void CorePlatformAndroid::OnGamepadTriggersAvailable(bool isAvailable)
{
    InputSystem::Instance()->GetGamepadDevice().OnTriggersAvailable(isAvailable);
}

void CorePlatformAndroid::OnInput(Vector<UIEvent>& allInputs)
{
    DVASSERT(!allInputs.empty());
    if (!allInputs.empty())
    {
        for (auto& e : allInputs)
        {
            UIControlSystem::Instance()->OnInput(&e);
        }
    }
}

bool CorePlatformAndroid::IsMultitouchEnabled()
{
    return InputSystem::Instance()->GetMultitouchEnabled();
}

bool CorePlatformAndroid::DownloadHttpFile(const String& url, const String& documentsPathname)
{
    if (androidDelegate)
    {
        FilePath path(documentsPathname);
        return androidDelegate->DownloadHttpFile(url, path.GetAbsolutePathname());
    }

    return false;
}

AndroidSystemDelegate* CorePlatformAndroid::GetAndroidSystemDelegate() const
{
    return androidDelegate;
}

void CorePlatformAndroid::SetNativeWindow(void* nativeWindow)
{
    rendererParams.window = nativeWindow;
}

int32 CorePlatformAndroid::GetViewWidth() const
{
    return pendingWidth;
}

int32 CorePlatformAndroid::GetViewHeight() const
{
    return pendingHeight;
}
}
#endif // #if defined(__DAVAENGINE_ANDROID__)
#endif // !__DAVAENGINE_COREV2__