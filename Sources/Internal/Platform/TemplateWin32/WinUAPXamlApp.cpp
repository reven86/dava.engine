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
#include "Platform/DeviceInfo.h"

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
using namespace ::Windows::Phone::UI::Input;

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

Windows::Foundation::Size WinUAPXamlApp::GetCurrentScreenSize()
{
    return Windows::Foundation::Size(windowWidth, windowHeight);
}

void WinUAPXamlApp::SetCursorState(bool isShown)
{
    // will be started on UI thread
    Logger::FrameworkDebug("[CorePlatformWinUAP] CursorState %d", static_cast<int32>(isShown));
    if (isShown != isMouseCursorShown)
    {
        if (isShown)
        {
            // Cursor showing disables relative mouse movement tracking
            Logger::FrameworkDebug("[CorePlatformWinUAP] ShowCursor");
            if (!isMouseCursorShown)
            {
                // Protect case where there isn't a window associated with the current thread.
                // This happens on initialization or when being called from a background thread.
                Window::Current->CoreWindow->PointerCursor = ref new CoreCursor(CoreCursorType::Arrow, 0);
                isMouseCursorShown = true;
            }
        }
        else
        {
            Logger::FrameworkDebug("[CorePlatformWinUAP] HideCursor");
            // Cursor hiding enables relative mouse movement tracking
            if (isMouseCursorShown)
            {
                // Protect case where there isn't a window associated with the current thread.
                // This happens on initialization or when being called from a background thread.
                Window::Current->CoreWindow->PointerCursor = nullptr;
                isMouseCursorShown = false;
            }
        }
    }
}

void WinUAPXamlApp::OnLaunched(::Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ args)
{
    CoreWindow^ coreWindow = Window::Current->CoreWindow;
    DeviceInfo::InitializeScreenInfo(static_cast<int32>(coreWindow->Bounds.Width), static_cast<int32>(coreWindow->Bounds.Height));
    uiThreadDispatcher = coreWindow->Dispatcher;

    SetupEventHandlers();
    CreateBaseXamlUI();

    UpdateScreenSize(coreWindow->Bounds.Width, coreWindow->Bounds.Height);
    InitRender();

    WorkItemHandler^ workItemHandler = ref new WorkItemHandler([this](Windows::Foundation::IAsyncAction^ action) { Run(); });
    renderLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);

    Window::Current->Activate();
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
    ReInitRender();
    InitCoordinatesSystem();

    // View size and orientation option should be configured in FrameowrkDidLaunched
    FrameworkDidLaunched();

    core->RunOnUIThreadBlocked([this]() {
        SetTitleName();
        InitInput();
        PrepareScreenSize();
        SetDisplayOrientations();
    });

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
    // Propagate to main thread
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
    // will be started on main thread
    PointerPoint^ pointPtr = args->CurrentPoint;
    PointerPointProperties^ pointProperties = pointPtr->Properties;
    PointerDeviceType type = args->CurrentPoint->PointerDevice->PointerDeviceType;
    if ((PointerDeviceType::Mouse == type) || (PointerDeviceType::Pen == type))
    {
        //update state before create dava event
        isLeftButtonPressed = pointProperties->IsLeftButtonPressed;
        isRightButtonPressed = pointProperties->IsRightButtonPressed;;
        isMiddleButtonPressed = pointProperties->IsMiddleButtonPressed;
    }
    DAVATouchEvent(UIEvent::PHASE_BEGAN, args->CurrentPoint->Position, args->CurrentPoint->PointerId);
}

void WinUAPXamlApp::OnPointerReleased(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ args)
{
    // will be started on main thread
    PointerDeviceType type = args->CurrentPoint->PointerDevice->PointerDeviceType;
    if ((PointerDeviceType::Mouse == type) || (PointerDeviceType::Pen == type))
    {
        if (isLeftButtonPressed || isMiddleButtonPressed || isRightButtonPressed)
        {
            DAVATouchEvent(UIEvent::PHASE_ENDED, args->CurrentPoint->Position, args->CurrentPoint->PointerId);
            PointerPointProperties^ pointProperties = args->CurrentPoint->Properties;
            // update state after create davaEvent
            isLeftButtonPressed = pointProperties->IsLeftButtonPressed;
            isRightButtonPressed = pointProperties->IsRightButtonPressed;
            isMiddleButtonPressed = pointProperties->IsMiddleButtonPressed;
        }
        else
        {
            DVASSERT(isLeftButtonPressed || isMiddleButtonPressed || isRightButtonPressed);
        }
    }
    else //  PointerDeviceType::Touch == args->CurrentPoint->PointerDevice->PointerDeviceType
    {
        DAVATouchEvent(UIEvent::PHASE_ENDED, args->CurrentPoint->Position, args->CurrentPoint->PointerId);
    }
}

void WinUAPXamlApp::OnPointerMoved(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ args)
{
    // will be started on main thread
    PointerDeviceType type = args->CurrentPoint->PointerDevice->PointerDeviceType;
    if ((PointerDeviceType::Mouse == type) || (PointerDeviceType::Pen == type))
    {
        if (isLeftButtonPressed || isMiddleButtonPressed || isRightButtonPressed)
        {
            DAVATouchEvent(UIEvent::PHASE_DRAG, args->CurrentPoint->Position, args->CurrentPoint->PointerId);
        }
        else
        {
            DAVATouchEvent(UIEvent::PHASE_MOVE, args->CurrentPoint->Position, args->CurrentPoint->PointerId);
        }
    }
    else //  PointerDeviceType::Touch == type
    {
        DAVATouchEvent(UIEvent::PHASE_DRAG, args->CurrentPoint->Position, args->CurrentPoint->PointerId);
    }
}

void WinUAPXamlApp::OnPointerEntered(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ args)
{
    // will be started on main thread
    Logger::FrameworkDebug("[CorePlatformWinUAP] OnPointerEntered");
    PointerDeviceType type = args->CurrentPoint->PointerDevice->PointerDeviceType;
    if (PointerDeviceType::Mouse == type)
    {
        if (isMouseCursorShown)
        {
            core->RunOnUIThread([this]() { SetCursorState(false); });
            isMouseCursorShown = false;
        }
    }
}

void WinUAPXamlApp::OnPointerExited(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ args)
{
    // will be started on main thread
    Logger::FrameworkDebug("[CorePlatformWinUAP] OnPointerExited");
    PointerDeviceType type = args->CurrentPoint->PointerDevice->PointerDeviceType;
    if ((PointerDeviceType::Mouse == type) || PointerDeviceType::Pen == type)
    {
        if (isLeftButtonPressed || isMiddleButtonPressed || isRightButtonPressed)
        {
            DAVATouchEvent(UIEvent::PHASE_ENDED, args->CurrentPoint->Position, args->CurrentPoint->PointerId);
            PointerPointProperties^ pointProperties = args->CurrentPoint->Properties;
            // update state after create davaEvent
            isLeftButtonPressed = pointProperties->IsLeftButtonPressed;
            isRightButtonPressed = pointProperties->IsRightButtonPressed;
            isMiddleButtonPressed = pointProperties->IsMiddleButtonPressed;
        }
        if (!isMouseCursorShown)
        {
            core->RunOnUIThread([this]() { SetCursorState(true); });
            isMouseCursorShown = true;
        }
    }
    else //  PointerDeviceType::Touch == type
    {
        DAVATouchEvent(UIEvent::PHASE_DRAG, args->CurrentPoint->Position, args->CurrentPoint->PointerId);
    }
}

void WinUAPXamlApp::OnPointerWheel(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ args)
{
    // will be started on main thread
    Logger::FrameworkDebug("[CorePlatformWinUAP] OnPointerWheel");
    Vector<DAVA::UIEvent> touches;
    PointerPoint^ point = args->CurrentPoint;
    PointerPointProperties^ pointProperties = point->Properties;
    int32 wheelDelta = pointProperties->MouseWheelDelta;
    UIEvent newTouch;
    newTouch.tid = 0;
    newTouch.physPoint.x = 0;
    newTouch.physPoint.y = static_cast<float32>(wheelDelta / WHEEL_DELTA);
    newTouch.phase = UIEvent::PHASE_WHEEL;
    touches.push_back(newTouch);
    UIControlSystem::Instance()->OnInput(UIEvent::PHASE_WHEEL, touches, allTouches);
}

void WinUAPXamlApp::OnPointerCaptureLost(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ args)
{
    Logger::FrameworkDebug("[CorePlatformWinUAP] OnPointerCaptureLost");

}

void WinUAPXamlApp::OnHardwareBackButtonPressed(_In_ Platform::Object^ sender, Windows::Phone::UI::Input::BackPressedEventArgs ^args)
{
    // Note: must run on main thread
    args->Handled = true;
    core->RunOnMainThread([this]() {
        Logger::FrameworkDebug("[CorePlatformWinUAP] OnHardwareBackButtonPressed");
    });
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
    VirtualKey key = args->VirtualKey;
    // Note: should be propagated to main thread
    core->RunOnMainThread([this, key]() {
        UIEvent ev;
        ev.keyChar = 0;
        ev.tapCount = 1;
        ev.phase = UIEvent::PHASE_KEYCHAR;
        ev.tid = InputSystem::Instance()->GetKeyboard().GetDavaKeyForSystemKey(static_cast<int32>(key));

        Vector<UIEvent> touches = { ev };
        UIControlSystem::Instance()->OnInput(0, touches, allTouches);
        touches.pop_back();
        UIControlSystem::Instance()->OnInput(0, touches, allTouches);
        InputSystem::Instance()->GetKeyboard().OnSystemKeyPressed(static_cast<int32>(key));
    });

}

void WinUAPXamlApp::OnKeyUp(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args)
{
    // Note: should be propagated to main thread
    VirtualKey key = args->VirtualKey;
    core->RunOnMainThread([this, key]() {
        InputSystem::Instance()->GetKeyboard().OnSystemKeyUnpressed(static_cast<int32>(key));
    });
}

void WinUAPXamlApp::OnMouseMoved(_In_ MouseDevice^ mouseDevice, _In_ MouseEventArgs^ args)
{
    // Note: must run on main thread
    Point position(static_cast<float32>(args->MouseDelta.X), static_cast<float32>(args->MouseDelta.Y));
    int32 button = 0;
    if (isLeftButtonPressed)
    {
        button = 1;
    }
    else if (isRightButtonPressed)
    {
        button = 2;
    }
    else if (isMiddleButtonPressed)
    {
        button = 3;
    }
    core->RunOnMainThread([this, position, button]() {
        if (isLeftButtonPressed || isMiddleButtonPressed || isRightButtonPressed)
        {
            DAVATouchEvent(UIEvent::PHASE_DRAG, position, button);
        }
        else
        {
            DAVATouchEvent(UIEvent::PHASE_MOVE, position, button);
        }
    });
}

void WinUAPXamlApp::DAVATouchEvent(UIEvent::eInputPhase phase, Windows::Foundation::Point position, int32 id)
{
    Logger::FrameworkDebug("[CorePlatformWinUAP] DAVATouchEvent phase = %d, ID = %d, position.X = %f, position.Y = %f", phase, id, position.X, position.Y);
    Vector<DAVA::UIEvent> touches;
    bool isFind = false;
    for (auto it = allTouches.begin(), end = allTouches.end(); it != end; ++it)
    {
        if (it->tid == id)
        {
            isFind = true;
            it->physPoint.x = position.X;
            it->physPoint.y = position.Y;
            it->phase = phase;
            break;
        }
    }
    if (!isFind)
    {
        UIEvent newTouch;
        newTouch.tid = id;
        newTouch.physPoint.x = position.X;
        newTouch.physPoint.y = position.Y;
        newTouch.phase = phase;
        allTouches.push_back(newTouch);
    }
    for (auto it = allTouches.begin(), end = allTouches.end(); it != end; ++it)
    {
        touches.push_back(*it);
    }
    if (phase == UIEvent::PHASE_ENDED)
    {
        for (Vector<DAVA::UIEvent>::iterator it = allTouches.begin(); it != allTouches.end(); ++it)
        {
            if (it->tid == id)
            {
                allTouches.erase(it);
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
    coreWindow->SizeChanged += ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &WinUAPXamlApp::OnWindowSizeChanged);
    coreWindow->VisibilityChanged += ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &WinUAPXamlApp::OnWindowVisibilityChanged);

    coreWindow->KeyDown += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &WinUAPXamlApp::OnKeyDown);
    coreWindow->KeyUp += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &WinUAPXamlApp::OnKeyUp);
    MouseDevice::GetForCurrentView()->MouseMoved += ref new TypedEventHandler<MouseDevice^, MouseEventArgs^>(this, &WinUAPXamlApp::OnMouseMoved);
    if (Windows::Foundation::Metadata::ApiInformation::IsTypePresent("Windows.Phone.UI.Input.HardwareButtons"))
    {
        HardwareButtons::BackPressed += ref new EventHandler<BackPressedEventArgs^>(this, &WinUAPXamlApp::OnHardwareBackButtonPressed);
        isPhoneApiDetect = true;
    }
}

void WinUAPXamlApp::SetupRenderLoopEventHandlers()
{
    mainThreadInputSource = swapChainPanel->CreateCoreIndependentInputSource(CoreInputDeviceTypes::Mouse | CoreInputDeviceTypes::Touch | CoreInputDeviceTypes::Pen);
    mainThreadInputSource->PointerPressed += ref new TypedEventHandler<Platform::Object^, PointerEventArgs^>(this, &WinUAPXamlApp::OnPointerPressed);
    mainThreadInputSource->PointerMoved += ref new TypedEventHandler<Platform::Object^, PointerEventArgs^>(this, &WinUAPXamlApp::OnPointerMoved);
    mainThreadInputSource->PointerReleased += ref new TypedEventHandler<Platform::Object^, PointerEventArgs^>(this, &WinUAPXamlApp::OnPointerReleased);
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
    Logger::FrameworkDebug("[CorePlatformWinUAP] SetTitleName");
    KeyedArchive* options = Core::Instance()->GetOptions();
    if (nullptr != options)
    {
        WideString title = StringToWString(options->GetString("title", "[set application title using core options property 'title']"));
        ApplicationView::GetForCurrentView()->Title = ref new ::Platform::String(title.c_str());
    }
}

void WinUAPXamlApp::SetDisplayOrientations()
{
    // Note: must run on UI thread
    Logger::FrameworkDebug("[CorePlatformWinUAP] SetDisplayOrientations");
    Core::eScreenOrientation orientMode = Core::Instance()->GetScreenOrientation();
    switch (orientMode)
    {
    case DAVA::Core::SCREEN_ORIENTATION_TEXTURE:
        break;
    case DAVA::Core::SCREEN_ORIENTATION_LANDSCAPE_RIGHT:
        displayOrientation = DisplayOrientations::Landscape;
        break;
    case DAVA::Core::SCREEN_ORIENTATION_LANDSCAPE_LEFT:
        displayOrientation = DisplayOrientations::LandscapeFlipped;
        break;
    case DAVA::Core::SCREEN_ORIENTATION_PORTRAIT:
        displayOrientation = DisplayOrientations::Portrait;
        break;
    case DAVA::Core::SCREEN_ORIENTATION_PORTRAIT_UPSIDE_DOWN:
        displayOrientation = DisplayOrientations::PortraitFlipped;
        break;
    case DAVA::Core::SCREEN_ORIENTATION_LANDSCAPE_AUTOROTATE:
        displayOrientation = DisplayOrientations::Landscape | DisplayOrientations::LandscapeFlipped;
        break;
    case DAVA::Core::SCREEN_ORIENTATION_PORTRAIT_AUTOROTATE:
        displayOrientation = DisplayOrientations::Portrait | DisplayOrientations::PortraitFlipped;
        break;
    }
    DisplayInformation::GetForCurrentView()->AutoRotationPreferences = displayOrientation;
}

void WinUAPXamlApp::InitInput()
{
    // Detect touch
    Logger::FrameworkDebug("[CorePlatformWinUAP] InitInput");
    TouchCapabilities^ touchCapabilities = ref new TouchCapabilities();
    isTouchDetected = (1 == touchCapabilities->TouchPresent);   // Touch is always present in MSVS simulator

    // Detect mouse
    MouseCapabilities^ mouseCapabilities = ref new MouseCapabilities();
    isMouseDetected = (1 == mouseCapabilities->MousePresent);
}

void WinUAPXamlApp::InitRender()
{
    Logger::FrameworkDebug("[CorePlatformWinUAP] InitRender");
    RenderManager::Create(Core::RENDERER_OPENGL_ES_2_0);
    RenderManager::Instance()->Create(swapChainPanel);
}

void WinUAPXamlApp::ReInitRender()
{
    Logger::FrameworkDebug("[CorePlatformWinUAP] ReInitRender");
    RenderManager::Instance()->Init(static_cast<int32>(windowWidth), static_cast<int32>(windowHeight));
    RenderSystem2D::Instance()->Init();
}

void WinUAPXamlApp::InitCoordinatesSystem()
{
    Logger::FrameworkDebug("[CorePlatformWinUAP] InitCoordinatesSystem");
    VirtualCoordinatesSystem::Instance()->SetInputScreenAreaSize(static_cast<int32>(windowWidth), static_cast<int32>(windowHeight));
    VirtualCoordinatesSystem::Instance()->SetPhysicalScreenSize(static_cast<int32>(windowWidth), static_cast<int32>(windowHeight));
    VirtualCoordinatesSystem::Instance()->EnableReloadResourceOnResize(true);
}

void WinUAPXamlApp::ReInitCoordinatesSystem()
{
    Logger::FrameworkDebug("[CorePlatformWinUAP] ReInitCoordinatesSystem");
    int32 intWidth = static_cast<int32>(windowWidth);
    int32 intHeight = static_cast<int32>(windowHeight);
    VirtualCoordinatesSystem::Instance()->SetInputScreenAreaSize(intWidth, intHeight);
    VirtualCoordinatesSystem::Instance()->UnregisterAllAvailableResourceSizes();
    VirtualCoordinatesSystem::Instance()->RegisterAvailableResourceSize(static_cast<int32>(windowWidth), static_cast<int32>(windowHeight), "Gfx");
    VirtualCoordinatesSystem::Instance()->SetPhysicalScreenSize(intWidth, intHeight);
    VirtualCoordinatesSystem::Instance()->SetVirtualScreenSize(intWidth, intHeight);
    VirtualCoordinatesSystem::Instance()->ScreenSizeChanged();
}

void WinUAPXamlApp::PrepareScreenSize()
{
    // Note: must run on UI thread
    KeyedArchive* options = Core::Instance()->GetOptions();
    if (nullptr != options)
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
        if (!isPhoneApiDetect)
        {
            SetPreferredSize(windowedMode.width, windowedMode.height);
        }
    }
}

void WinUAPXamlApp::UpdateScreenSize(float32 width, float32 height)
{
    windowWidth = width;
    windowHeight = height;
    Logger::FrameworkDebug("[CorePlatformWinUAP] UpdateScreenSize windowWidth = %f, windowHeight = %f.", windowWidth, windowHeight);
}

void WinUAPXamlApp::SetFullScreen(bool isFullscreen_)
{
    // Note: must run on UI thread
    Logger::FrameworkDebug("[CorePlatformWinUAP] SetFullScreen %d", (int32)isFullscreen_);
    ApplicationView^ view = ApplicationView::GetForCurrentView();
    bool isFull = view->IsFullScreenMode;
    if (isFull == isFullscreen_)
    {
        return;
    }
    if (isFullscreen_)
    {
        bool res = view->TryEnterFullScreenMode();
        DVASSERT(res);
    }
    else
    {
        if (!isPhoneApiDetect)
        {
            view->ExitFullScreenMode();
        }
    }
}

void WinUAPXamlApp::SetPreferredSize(int32 width, int32 height)
{
    // Note: must run on UI thread
    Logger::FrameworkDebug("[CorePlatformWinUAP] SetPreferredSize width = %d, height = %d", width, height);
    // MSDN::This property only has an effect when the app is launched on a desktop device that is not in tablet mode.
    ApplicationView::GetForCurrentView()->PreferredLaunchViewSize = Windows::Foundation::Size(static_cast<float32>(width), static_cast<float32>(height));
}

}   // namespace DAVA

#endif  // __DAVAENGINE_WIN_UAP__
