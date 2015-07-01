/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "Base/Platform.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Core/Core.h"
#include "Render/RenderManager.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "UI/UIScreenManager.h"

#include "Platform/SystemTimer.h"
#include "Platform/TemplateWin32/CorePlatformWinUAP.h"
#include "Platform/TemplateWin32/WinUAPXamlApp.h"

#include "FileSystem/Logger.h"

#include "Utils/Utils.h"

#include "WinUAPXamlApp.h"

extern void FrameworkDidLaunched();
extern void FrameworkWillTerminate();

using namespace ::Windows::System;
using namespace ::Windows::Foundation;
using namespace ::Windows::UI::Core;
using namespace ::Windows::UI::Xaml;
using namespace ::Windows::UI::Xaml::Controls;
using namespace ::Windows::UI::Input;
using namespace ::Windows::UI::ViewManagement;
using namespace ::Windows::Devices::Input;
using namespace ::Windows::ApplicationModel;
using namespace ::Windows::Graphics::Display;
using namespace ::Windows::ApplicationModel::Core;
using namespace ::Windows::UI::Xaml::Media;
using namespace ::Windows::System::Threading;

namespace DAVA
{

WinUAPXamlApp::WinUAPXamlApp()
    : core(static_cast<CorePlatformWinUAP*>(Core::Instance()))
{}

DisplayOrientations WinUAPXamlApp::GetDisplayOrientation()
{
    return displayOrientation;
}

ApplicationViewWindowingMode WinUAPXamlApp::GetScreenMode()
{
    return isFullscreen ? ApplicationViewWindowingMode::FullScreen :
                          ApplicationViewWindowingMode::PreferredLaunchViewSize;
}

void WinUAPXamlApp::SetScreenMode(ApplicationViewWindowingMode screenMode)
{
    // Note: must run on UI thread
    bool fullscreen = ApplicationViewWindowingMode::FullScreen == screenMode;
    SetFullScreen(fullscreen);
}

void WinUAPXamlApp::OnLaunched(::Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ args)
{
    CoreWindow^ coreWindow = Window::Current->CoreWindow;
    uiThreadDispatcher = coreWindow->Dispatcher;

    SetupEventHandlers();
    CreateBaseXamlUI();

    UpdateScreenSize(coreWindow->Bounds.Width, coreWindow->Bounds.Height);
    Window::Current->Activate();

    RenderManager::Create(Core::RENDERER_OPENGL_ES_2_0);
    RenderManager::Instance()->Create(swapChainPanel);

    SetDisplayOrientations(Core::eScreenOrientation::SCREEN_ORIENTATION_LANDSCAPE_AUTOROTATE);
    InitInput();
    PrepareScreenSize();

    WorkItemHandler^ workItemHandler = ref new WorkItemHandler([this](Windows::Foundation::IAsyncAction^ action) { Run(); });
    renderLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
}

void WinUAPXamlApp::AddUIElement(Windows::UI::Xaml::UIElement^ uiElement)
{
    // Note: must be called from UI thread
    canvas->Children->Append(uiElement);
}

void WinUAPXamlApp::RemoveUIElement(Windows::UI::Xaml::UIElement^ uiElement)
{
    // Note: must be called from UI thread
    unsigned int index = 0;
    for (auto x = canvas->Children->First();x->HasCurrent;x->MoveNext(), ++index)
    {
        if (x->Current == uiElement)
        {
            canvas->Children->RemoveAt(index);
            break;
        }
    }
}

void WinUAPXamlApp::PositionUIElement(Windows::UI::Xaml::UIElement^ uiElement, float32 x, float32 y)
{
    // Note: must be called from UI thread
    canvas->SetLeft(uiElement, x);
    canvas->SetTop(uiElement, y);
}

void WinUAPXamlApp::Run()
{
    Core::Instance()->CreateSingletons();

    SetupRenderLoopEventHandlers();

    RenderManager::Instance()->BindToCurrentThread();
    RenderManager::Instance()->Init(integralWindowWidth, integralWindowHeight);
    RenderSystem2D::Instance()->Init();

    InitCoordinatesSystem();
    FrameworkDidLaunched();

    core->RunOnUIThread([this]() { SetTitleName(); });

    Core::Instance()->SystemAppStarted();
    while (!quitFlag)
    {
        mainThreadInputSource->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

        DAVA::uint64 startTime = DAVA::SystemTimer::Instance()->AbsoluteMS();
        RenderManager::Instance()->Lock();
        Core::Instance()->SystemProcessFrame();
        RenderManager::Instance()->Unlock();
        uint32 elapsedTime = (uint32)(SystemTimer::Instance()->AbsoluteMS() - startTime);
        int32 sleepMs = 1;
        int32 fps = RenderManager::Instance()->GetFPS();
        if (fps > 0)
        {
            sleepMs = (1000 / fps) - elapsedTime;
            if (sleepMs > 0)
            {
                Thread::Sleep(sleepMs);
            }
        }
    }

    ApplicationCore* appCore = Core::Instance()->GetApplicationCore();
    if (appCore != nullptr && appCore->OnQuit())
    {
        Application::Current->Exit();
    }

    Core::Instance()->SystemAppFinished();
    FrameworkWillTerminate();
    Core::Instance()->ReleaseSingletons();

    Application::Current->Exit();
}

void WinUAPXamlApp::OnSuspending(::Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ args)
{
    isWindowVisible = false;
    Core::Instance()->SetIsActive(isWindowVisible);
}

void WinUAPXamlApp::OnResuming(::Platform::Object^ sender, ::Platform::Object^ args)
{
    isWindowVisible = true;
    Core::Instance()->SetIsActive(isWindowVisible);
}

void WinUAPXamlApp::OnWindowActivationChanged(::Windows::UI::Core::CoreWindow^ sender, ::Windows::UI::Core::WindowActivatedEventArgs^ args)
{
    CoreWindowActivationState state = args->WindowActivationState;
    switch (state)
    {
    case CoreWindowActivationState::CodeActivated:
    case CoreWindowActivationState::PointerActivated:
        Core::Instance()->SetIsActive(true);
        break;
    case CoreWindowActivationState::Deactivated:
        Core::Instance()->SetIsActive(false);
        break;
    default:
        break;
    }
}

void WinUAPXamlApp::OnWindowVisibilityChanged(::Windows::UI::Core::CoreWindow^ sender, ::Windows::UI::Core::VisibilityChangedEventArgs^ args)
{
    isWindowVisible = args->Visible;
    Core::Instance()->SetIsActive(isWindowVisible);
}

void WinUAPXamlApp::OnWindowSizeChanged(::Windows::UI::Core::CoreWindow^ sender, ::Windows::UI::Core::WindowSizeChangedEventArgs^ args)
{
    // Propagate to main thread
    float32 w = args->Size.Width;
    float32 h = args->Size.Height;
    core->RunOnMainThread([this, w, h]() {
        UpdateScreenSize(w, h);
        ReInitRender();
        ReInitCoordinatesSystem();
        UIScreenManager::Instance()->ScreenSizeChanged();
    });
}

void WinUAPXamlApp::OnPointerPressed(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ args)
{
    PointerPoint^ curPoint = args->CurrentPoint;
    {
        PointerPointProperties^ pointProperties = curPoint->Properties;
        isLeftButtonPressed = pointProperties->IsLeftButtonPressed;
        isRightButtonPressed = pointProperties->IsRightButtonPressed;;
        isMiddleButtonPressed = pointProperties->IsMiddleButtonPressed;
    }
    DAVATouchEvent(UIEvent::PHASE_BEGAN, curPoint);
}

void WinUAPXamlApp::OnPointerReleased(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ args)
{
    PointerPoint^ curPoint = args->CurrentPoint;
    DAVATouchEvent(UIEvent::PHASE_ENDED, curPoint);
    {
        PointerPointProperties^ pointProperties = curPoint->Properties;
        isLeftButtonPressed = pointProperties->IsLeftButtonPressed;
        isRightButtonPressed = pointProperties->IsRightButtonPressed;;
        isMiddleButtonPressed = pointProperties->IsMiddleButtonPressed;
    }
}

void WinUAPXamlApp::OnPointerMoved(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ args)
{
    PointerPoint^ curPoint = args->CurrentPoint;
    switch (curPoint->PointerDevice->PointerDeviceType)
    {
    case PointerDeviceType::Touch:
        DAVATouchEvent(UIEvent::PHASE_DRAG, curPoint);
        break;
    case PointerDeviceType::Mouse:
        if (curPoint->Properties->IsLeftButtonPressed)
        {
            DAVATouchEvent(UIEvent::PHASE_DRAG, curPoint);
        }
        break;
    case PointerDeviceType::Pen:
        break;
    default:
        break;
    }
}

void WinUAPXamlApp::OnPointerEntered(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ args)
{
    
}

void WinUAPXamlApp::OnPointerExited(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ args)
{
    if (!isMouseCursorShown)
    {
        core->RunOnUIThread([this]() { ShowCursor(); });
        isMouseCursorShown = true;
    }
}

void WinUAPXamlApp::OnPointerWheel(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ args)
{
    PointerPoint^ curPoint = args->CurrentPoint;
    PointerPointProperties^ pointProperties = curPoint->Properties;
    int32 wheelDelta = pointProperties->MouseWheelDelta;

    UIEvent ev;
    ev.tid = 0;
    ev.physPoint.x = 0;
    ev.physPoint.y = static_cast<float32>(wheelDelta) / static_cast<float32>(WHEEL_DELTA);
    ev.phase = UIEvent::PHASE_WHEEL;

    Vector<UIEvent> touches{ev};
    UIControlSystem::Instance()->OnInput(UIEvent::PHASE_WHEEL, touches, allTouches);
}

void WinUAPXamlApp::OnPointerCaptureLost(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ args)
{
    
}

void WinUAPXamlApp::OnKeyDown(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args)
{
    auto window = CoreWindow::GetForCurrentThread();
    if (window)
    {
        CoreVirtualKeyStates menuStatus = window->GetKeyState(VirtualKey::Menu);
        CoreVirtualKeyStates tabStatus = window->GetKeyState(VirtualKey::Tab);
        bool isPressOrLock = static_cast<bool>((menuStatus & CoreVirtualKeyStates::Down) & (tabStatus & CoreVirtualKeyStates::Down));
        if (isPressOrLock)
        {
            __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
        }
    }

    // Note: should be propagated to main thread

    UIEvent ev;
    ev.keyChar = 0;
    ev.tapCount = 1;
    ev.phase = UIEvent::PHASE_KEYCHAR;
    ev.tid = InputSystem::Instance()->GetKeyboard().GetDavaKeyForSystemKey(static_cast<int32>(args->VirtualKey));

    Vector<UIEvent> touches = {ev};
    UIControlSystem::Instance()->OnInput(0, touches, allTouches);
    touches.pop_back();
    UIControlSystem::Instance()->OnInput(0, touches, allTouches);
    InputSystem::Instance()->GetKeyboard().OnSystemKeyPressed(static_cast<int32>(args->VirtualKey));
}

void WinUAPXamlApp::OnKeyUp(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args)
{
    // Note: should be propagated to main thread
    InputSystem::Instance()->GetKeyboard().OnSystemKeyUnpressed(static_cast<int32>(args->VirtualKey));
}

void WinUAPXamlApp::DAVATouchEvent(UIEvent::eInputPhase phase, Windows::UI::Input::PointerPoint^ pointerPoint)
{
    Point position = pointerPoint->Position;

    int button = 0;
    if (isLeftButtonPressed)
        button = 1;
    else if (isRightButtonPressed)
        button = 2;
    else if (isMiddleButtonPressed)
        button = 3;
    bool isFound = false;
    for (auto& ev : allTouches)
    {
        if (ev.tid == button)
        {
            isFound = true;
            ev.physPoint.x = position.X;
            ev.physPoint.y = position.Y;
            ev.phase = phase;
            break;
        }
    }
    if (!isFound)
    {
        UIEvent ev;
        ev.tid = button;
        ev.physPoint.x = position.X;
        ev.physPoint.y = position.Y;
        ev.phase = phase;
        allTouches.push_back(ev);
    }

    Vector<UIEvent> touches;
    for (const auto& x : allTouches)
    {
        touches.push_back(x);
    }
    if (phase == UIEvent::PHASE_ENDED)
    {
        for (auto i = allTouches.begin(), e = allTouches.end();i != e;++i)
        {
            if (i->tid == button)
            {
                allTouches.erase(i);
                break;
            }
        }
    }
    UIControlSystem::Instance()->OnInput(phase, touches, allTouches);
}

void WinUAPXamlApp::SetupEventHandlers()
{
    Suspending += ref new SuspendingEventHandler(this, &WinUAPXamlApp::OnSuspending);
    Resuming += ref new EventHandler<::Platform::Object^>(this, &WinUAPXamlApp::OnResuming);

    CoreWindow^ coreWindow = Window::Current->CoreWindow;
    coreWindow->Activated += ref new TypedEventHandler<CoreWindow^, WindowActivatedEventArgs^>(this, &WinUAPXamlApp::OnWindowActivationChanged);
    coreWindow->VisibilityChanged += ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &WinUAPXamlApp::OnWindowVisibilityChanged);
    coreWindow->SizeChanged += ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &WinUAPXamlApp::OnWindowSizeChanged);

    coreWindow->KeyDown += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &WinUAPXamlApp::OnKeyDown);
    coreWindow->KeyUp += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &WinUAPXamlApp::OnKeyUp);
}

void WinUAPXamlApp::SetupRenderLoopEventHandlers()
{
    mainThreadInputSource = swapChainPanel->CreateCoreIndependentInputSource(CoreInputDeviceTypes::Mouse |
                                                                       CoreInputDeviceTypes::Touch |
                                                                       CoreInputDeviceTypes::Pen);

    mainThreadInputSource->PointerPressed += ref new TypedEventHandler<Platform::Object^, PointerEventArgs^>(this, &WinUAPXamlApp::OnPointerPressed);
    mainThreadInputSource->PointerReleased += ref new TypedEventHandler<Platform::Object^, PointerEventArgs^>(this, &WinUAPXamlApp::OnPointerReleased);
    mainThreadInputSource->PointerMoved += ref new TypedEventHandler<Platform::Object^, PointerEventArgs^>(this, &WinUAPXamlApp::OnPointerMoved);
    mainThreadInputSource->PointerEntered += ref new TypedEventHandler<Platform::Object^, PointerEventArgs^>(this, &WinUAPXamlApp::OnPointerEntered);
    mainThreadInputSource->PointerExited += ref new TypedEventHandler<Platform::Object^, PointerEventArgs^>(this, &WinUAPXamlApp::OnPointerExited);
    mainThreadInputSource->PointerWheelChanged += ref new TypedEventHandler<Platform::Object^, PointerEventArgs^>(this, &WinUAPXamlApp::OnPointerWheel);
}

void WinUAPXamlApp::CreateBaseXamlUI()
{
    swapChainPanel = ref new Controls::SwapChainPanel();
    canvas = ref new Controls::Canvas();
    swapChainPanel->Children->Append(canvas);
    Window::Current->Content = swapChainPanel;
}

void WinUAPXamlApp::SetTitleName()
{
    // Note: must run on UI thread
    KeyedArchive* options = Core::Instance()->GetOptions();
    if (options != nullptr)
    {
        WideString title = StringToWString(options->GetString("title", "[set application title using core options property 'title']"));
        ApplicationView::GetForCurrentView()->Title = ref new ::Platform::String(title.c_str());
    }
}

void WinUAPXamlApp::SetDisplayOrientations(Core::eScreenOrientation orientation)
{
    // Note: must run on UI thread
    DisplayInformation^ displayInfo = DisplayInformation::GetForCurrentView();
    switch (orientation)
    {
    case Core::SCREEN_ORIENTATION_TEXTURE:
        displayInfo->AutoRotationPreferences = DisplayOrientations::None;
        break;
    case Core::SCREEN_ORIENTATION_LANDSCAPE_RIGHT:
        displayInfo->AutoRotationPreferences = DisplayOrientations::Landscape;
        break;
    case Core::SCREEN_ORIENTATION_LANDSCAPE_LEFT:
        displayInfo->AutoRotationPreferences = DisplayOrientations::LandscapeFlipped;
        break;
    case Core::SCREEN_ORIENTATION_PORTRAIT:
        displayInfo->AutoRotationPreferences = DisplayOrientations::Portrait;
        break;
    case Core::SCREEN_ORIENTATION_PORTRAIT_UPSIDE_DOWN:
        displayInfo->AutoRotationPreferences = DisplayOrientations::PortraitFlipped;
        break;
    case Core::SCREEN_ORIENTATION_LANDSCAPE_AUTOROTATE:
        displayInfo->AutoRotationPreferences = DisplayOrientations::Landscape | DisplayOrientations::LandscapeFlipped;
        break;
    case Core::SCREEN_ORIENTATION_PORTRAIT_AUTOROTATE:
        displayInfo->AutoRotationPreferences = DisplayOrientations::Portrait | DisplayOrientations::PortraitFlipped;
        break;
    default:
        displayInfo->AutoRotationPreferences = DisplayOrientations::None;
        break;
    }
}

void WinUAPXamlApp::InitInput()
{
    // Note: must run on UI thread
    // Detect touch
    TouchCapabilities^ touchCapabilities = ref new TouchCapabilities();
    isTouchDetected = (1 == touchCapabilities->TouchPresent);   // Touch is always present in MSVS simulator

    // Detect mouse
    MouseCapabilities^ mouseCapabilities = ref new MouseCapabilities();
    isMouseDetected = (1 == mouseCapabilities->MousePresent);

    // TODO: do I really need this?
    UIViewSettings^ viewSettings = UIViewSettings::GetForCurrentView();
    userInteractionMode = viewSettings->UserInteractionMode;
}

void WinUAPXamlApp::InitRender()
{
    RenderManager::Create(Core::RENDERER_OPENGL_ES_2_0);
    RenderManager::Instance()->Create(swapChainPanel);
    RenderManager::Instance()->Init(integralWindowWidth, integralWindowHeight);
    RenderSystem2D::Instance()->Init();
}

void WinUAPXamlApp::ReInitRender()
{
    RenderManager::Instance()->Init(integralWindowWidth, integralWindowHeight);
    RenderSystem2D::Instance()->Init();
}

void WinUAPXamlApp::InitCoordinatesSystem()
{
    VirtualCoordinatesSystem::Instance()->SetInputScreenAreaSize(static_cast<int32>(windowWidth), static_cast<int32>(windowHeight));
    VirtualCoordinatesSystem::Instance()->SetPhysicalScreenSize(integralWindowWidth, integralWindowHeight);
    VirtualCoordinatesSystem::Instance()->EnableReloadResourceOnResize(true);
}

void WinUAPXamlApp::ReInitCoordinatesSystem()
{
    int32 intWidth = static_cast<int32>(windowWidth);
    int32 intHeight = static_cast<int32>(windowHeight);
    VirtualCoordinatesSystem::Instance()->SetInputScreenAreaSize(intWidth, intHeight);
    VirtualCoordinatesSystem::Instance()->UnregisterAllAvailableResourceSizes();
    VirtualCoordinatesSystem::Instance()->RegisterAvailableResourceSize(integralWindowWidth, integralWindowHeight, "Gfx");
    VirtualCoordinatesSystem::Instance()->SetPhysicalScreenSize(integralWindowWidth, integralWindowHeight);
    VirtualCoordinatesSystem::Instance()->SetVirtualScreenSize(intWidth, intHeight);
    VirtualCoordinatesSystem::Instance()->ScreenSizeChanged();
}

void WinUAPXamlApp::PrepareScreenSize()
{
    // Note: must run on UI thread
    KeyedArchive* options = Core::Instance()->GetOptions();
    if (options != nullptr)
    {
        windowedMode.width = options->GetInt32("width", DisplayMode::DEFAULT_WIDTH);
        windowedMode.height = options->GetInt32("height", DisplayMode::DEFAULT_HEIGHT);
        windowedMode.bpp = options->GetInt32("bpp", DisplayMode::DEFAULT_BITS_PER_PIXEL);

        fullscreenMode.width = options->GetInt32("fullscreen.width", fullscreenMode.width);
        fullscreenMode.height = options->GetInt32("fullscreen.height", fullscreenMode.height);
        fullscreenMode.bpp = windowedMode.bpp;
        isFullscreen = (0 != options->GetInt32("fullscreen", 0));
    }
    Logger::FrameworkDebug("[PlatformWin32] best display fullscreen mode matched: %d x %d x %d refreshRate: %d",
                           fullscreenMode.width, fullscreenMode.height, fullscreenMode.bpp, fullscreenMode.refreshRate);
    SetFullScreen(isFullscreen);
    if (isFullscreen)
    {
        currentMode = fullscreenMode;
    }
    else
    {
        currentMode = windowedMode;
        SetPreferredSize(windowedMode.width, windowedMode.height);
    }
}

void WinUAPXamlApp::UpdateScreenSize(float32 width, float32 height)
{
    windowWidth = width;
    windowHeight = height;

    integralWindowWidth = static_cast<int32>(windowWidth * rawPixelInViewPixel);
    integralWindowHeight = static_cast<int32>(windowHeight * rawPixelInViewPixel);
}

void WinUAPXamlApp::SetFullScreen(bool isFullscreen_)
{
    // Note: must run on UI thread
    ApplicationView^ appView = ApplicationView::GetForCurrentView();
    if (appView->IsFullScreenMode != isFullscreen_)
    {
        if (isFullscreen_)
        {
            appView->TryEnterFullScreenMode();
            appView->PreferredLaunchWindowingMode = ApplicationViewWindowingMode::FullScreen;
        }
        else
        {
            appView->ExitFullScreenMode();
            appView->PreferredLaunchWindowingMode = ApplicationViewWindowingMode::PreferredLaunchViewSize;
        }
    }
}

void WinUAPXamlApp::SetPreferredSize(int32 width, int32 height)
{
    // Note: must run on UI thread
    ApplicationView^ appView = ApplicationView::GetForCurrentView();
    // This property only has an effect when the app is launched on a desktop device that is not in tablet mode.
    appView->PreferredLaunchViewSize = Size(static_cast<float32>(width), static_cast<float32>(height));
}

void WinUAPXamlApp::ShowCursor()
{
    // Note: must run on UI thread
    // Cursor showing disables relative mouse movement tracking
    CoreWindow^ coreWindow = Window::Current->CoreWindow;
    coreWindow->PointerCursor = ref new CoreCursor(CoreCursorType::Arrow, 0);
}

void WinUAPXamlApp::HideCursor()
{
    // Note: must run on UI thread
    // Cursor hiding enables relative mouse movement tracking
    CoreWindow^ coreWindow = Window::Current->CoreWindow;
    coreWindow->PointerCursor = nullptr;
}

}   // namespace DAVA

#endif  // __DAVAENGINE_WIN_UAP__
