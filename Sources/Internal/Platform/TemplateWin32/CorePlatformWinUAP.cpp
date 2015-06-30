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

#include "Utils/Utils.h"
#include "Render/RenderManager.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Platform/SystemTimer.h"
#include "UI/UIControlSystem.h"
#include "UI/UIScreenManager.h"
#include "Input/InputSystem.h"
#include "Input/KeyboardDevice.h"
#include "Core/Core.h"

#include "CorePlatformWinUAP.h"
#include <angle_windowsstore.h>

extern void FrameworkDidLaunched();
extern void FrameworkWillTerminate();

namespace DAVA
{

using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::Devices::Input;
using namespace Windows::Foundation;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::Graphics::Display;
using namespace Windows::System;

//------------------------------------------------------------------------------------------------------
//                                      Core
//------------------------------------------------------------------------------------------------------
int Core::Run(int argc, char * argv[], AppHandle handle)
{
    CorePlatformWinStore* core = new CorePlatformWinStore();
    if (nullptr != core)
    {
        core->InitArgs();
        // creating in another thread
        // core->CreateSingletons();
        core->Run();
        // core->ReleaseSingletons();
    }
    return 0;
}

int Core::RunCmdTool(int argc, char * argv[], AppHandle handle)
{
    return 1;
}
//------------------------------------------------------------------------------------------------------
//                                      CorePlatformWinStore
//------------------------------------------------------------------------------------------------------
CorePlatformWinStore::CorePlatformWinStore()
{
}

CorePlatformWinStore::~CorePlatformWinStore()
{
}

void CorePlatformWinStore::InitArgs()
{
    SetCommandLine(WStringToString(::GetCommandLineW()));
}

void CorePlatformWinStore::Run()
{
    // create window
    auto winStoreApplicationSource = ref new WinStoreApplicationSource();
    CoreApplication::Run(winStoreApplicationSource);
    //get frame class after creation
    frameWinUAP = dynamic_cast<WinStoreFrame^>(winStoreApplicationSource->GetView());
}

Core::eScreenMode CorePlatformWinStore::GetScreenMode()
{
    ViewManagement::ApplicationViewWindowingMode viewMode = frameWinUAP->GetScreenMode();
    if (ViewManagement::ApplicationViewWindowingMode::FullScreen == viewMode)
    {
        return Core::eScreenMode::MODE_FULLSCREEN;
    }
    else if (ViewManagement::ApplicationViewWindowingMode::PreferredLaunchViewSize == viewMode)
    {
        return Core::eScreenMode::MODE_WINDOWED;
    }
    return Core::eScreenMode::MODE_UNSUPPORTED;
}

/*
// TODO::makkis:: wait response from Microsoft
void CorePlatformWinStore::GetAvailableDisplayModes(List<DisplayMode> & availableDisplayModes)
{
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
}
*/

void CorePlatformWinStore::SwitchScreenToMode(eScreenMode screenMode)
{
    if (screenMode == Core::MODE_FULLSCREEN)
    {
        frameWinUAP->SetScreenMode(ViewManagement::ApplicationViewWindowingMode::FullScreen);
    }
    else if (screenMode == Core::MODE_WINDOWED)
    {
        frameWinUAP->SetScreenMode(ViewManagement::ApplicationViewWindowingMode::PreferredLaunchViewSize);
    }
}

/*
// TODO::makkis:: wait response from Microsoft
DisplayMode CorePlatformWinStore::FindBestMode(const DisplayMode & requestedMode)
{
    return Core::FindBestMode(requestedMode);
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
}
*/

void CorePlatformWinStore::Quit()
{
    CoreApplication::Exit();
}

void CorePlatformWinStore::SetIcon(int32 iconId)
{
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
}

Core::eScreenOrientation CorePlatformWinStore::GetScreenOrientation()
{
    switch (frameWinUAP->GetDisplayOrientation())
    {
    case Windows::Graphics::Display::DisplayOrientations::None:
        return Core::eScreenOrientation::SCREEN_ORIENTATION_TEXTURE;
    case Windows::Graphics::Display::DisplayOrientations::Landscape:
        return Core::eScreenOrientation::SCREEN_ORIENTATION_LANDSCAPE_RIGHT;
    case Windows::Graphics::Display::DisplayOrientations::Portrait:
        return Core::eScreenOrientation::SCREEN_ORIENTATION_PORTRAIT;
    case Windows::Graphics::Display::DisplayOrientations::LandscapeFlipped:
        return Core::eScreenOrientation::SCREEN_ORIENTATION_LANDSCAPE_LEFT;
    case Windows::Graphics::Display::DisplayOrientations::PortraitFlipped:
        return Core::eScreenOrientation::SCREEN_ORIENTATION_TEXTURE;
    }
    return Core::eScreenOrientation::SCREEN_ORIENTATION_TEXTURE;
}

uint32 CorePlatformWinStore::GetScreenDPI()
{
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
    return 1;
}

void CorePlatformWinStore::GoBackground(bool isLock)
{
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
}

void CorePlatformWinStore::GoForeground()
{
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
}
//------------------------------------------------------------------------------------------------------
//                          WinStoreApplicationSource
//------------------------------------------------------------------------------------------------------
IFrameworkView^ WinStoreApplicationSource::CreateView()
{
    frame = ref new WinStoreFrame();
    return frame.Get();
}
//------------------------------------------------------------------------------------------------------
//                          WinStoreFrame
//------------------------------------------------------------------------------------------------------
WinStoreFrame::WinStoreFrame()
{
    isMouseDetected = false;
    isTouchDetected = false;
    ViewManagement::UserInteractionMode userInteractionMode = ViewManagement::UserInteractionMode::Mouse;
    DisplayOrientations displayOrientation = DisplayOrientations::None;
    willQuit = false;
    isWindowClosed = false;
    isWindowVisible = false;
    isFullscreen = false;

    windowWidth = static_cast<float32>(DISPLAY_MODE_DEFAULT_WIDTH);
    windowHeight = static_cast<float32>(DISPLAY_MODE_DEFAULT_HEIGHT);
    rawPixelInViewPixel = 1.0;
    exWindowWidth = static_cast<int32>(windowWidth * rawPixelInViewPixel);
    exWindowHeight = static_cast<int32>(windowHeight * rawPixelInViewPixel);

    //mouse properties
    isMouseCursorShown = false;
    isRightButtonPressed = false;
    isLeftButtonPressed = false;
    isMiddleButtonPressed = false;

    windowedMode = DisplayMode(DISPLAY_MODE_DEFAULT_WIDTH, DISPLAY_MODE_DEFAULT_HEIGHT, DISPLAY_MODE_DEFAULT_BITS_PER_PIXEL, DISPLAY_MODE_DEFAULT_DISPLAYFREQUENCY);
    fullscreenMode = currentMode = windowedMode;
    //windowPositionBeforeFullscreen;
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
}

// This method is called on application launch.
void WinStoreFrame::Initialize(_In_ CoreApplicationView^ applicationView)
{
    //AppDisplayInfo^ appDisplayInfo = AppInfo::DisplayInfo;
    //Platform::String str = ApplicationModel::AppDisplayInfo::DisplayName;

    // because other thread
    CorePlatformWinStore::Instance()->CreateSingletons();
    
    if (nullptr == applicationView)
    {
        return;
    }
    applicationView->Activated += ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &WinStoreFrame::OnActivated);
    CoreApplication::Suspending += ref new EventHandler<SuspendingEventArgs^>(this, &WinStoreFrame::OnSuspending);
    CoreApplication::Resuming += ref new EventHandler<Platform::Object^>(this, &WinStoreFrame::OnResuming);

}

// This method is called after Initialize.
void WinStoreFrame::SetWindow(_In_ CoreWindow^ window)
{
    if (nullptr == window)
    {
        return;
    }

    ownWindow = window;
    // Specify the cursor type as the standard arrow cursor.
    ShowCursor();
    // Allow the application to respond when the window size changes.
    ownWindow->Activated += ref new TypedEventHandler<CoreWindow^, WindowActivatedEventArgs^>(this, &WinStoreFrame::OnWindowActivationChanged);
    ownWindow->SizeChanged += ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &WinStoreFrame::OnWindowSizeChanged);
    ownWindow->Closed += ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &WinStoreFrame::OnWindowClosed);
    ownWindow->VisibilityChanged += ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &WinStoreFrame::OnVisibilityChanged);

    DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();
    if (nullptr != currentDisplayInformation)
    {
        rawPixelInViewPixel = currentDisplayInformation->RawPixelsPerViewPixel;
        currentDisplayInformation->DpiChanged += ref new TypedEventHandler<DisplayInformation^, Platform::Object^>(this, &WinStoreFrame::OnDpiChanged);
        currentDisplayInformation->OrientationChanged += ref new TypedEventHandler<DisplayInformation^, Platform::Object^>(this, &WinStoreFrame::OnOrientationChanged);
//        currentDisplayInformation->StereoEnabledChanged += ref new TypedEventHandler<DisplayInformation^, Platform::Object^>(this, &WinStoreFrame::OnStereoEnabledChanged);
        DisplayInformation::DisplayContentsInvalidated += ref new TypedEventHandler<DisplayInformation^, Platform::Object^>(this, &WinStoreFrame::OnDisplayContentsInvalidated);
    }

    // input
    ownWindow->PointerPressed += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &WinStoreFrame::OnPointerPressed);
    ownWindow->PointerMoved += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &WinStoreFrame::OnPointerMoved);
    ownWindow->PointerReleased += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &WinStoreFrame::OnPointerReleased);
    ownWindow->PointerExited += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &WinStoreFrame::OnPointerExited);
    ownWindow->KeyDown += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &WinStoreFrame::OnKeyDown);
    ownWindow->KeyUp += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &WinStoreFrame::OnKeyUp);
    ownWindow->PointerWheelChanged += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &WinStoreFrame::OnWheel);
    // There is a separate handler for mouse only relative mouse movement events.
    // MouseDevice::GetForCurrentView()->MouseMoved += ref new TypedEventHandler<MouseDevice^, MouseEventArgs^>(this, &WinStoreFrame::OnMouseMoved);
    UpdateScreenSize(window->Bounds.Width, window->Bounds.Height);
}

void WinStoreFrame::Load(_In_ Platform::String^ entryPoint)
{
}

// This method is called after Load.
void WinStoreFrame::Run()
{
    willQuit = false;
    isWindowClosed = false;
    isWindowVisible = true;
    SetTitleName();
    InitInput();
    InitRender();
    InitCoordinatesSystem();

    PreparationSizeScreen();
    SetDisplayOrientations(Core::eScreenOrientation::SCREEN_ORIENTATION_LANDSCAPE_AUTOROTATE);
    
    FrameworkDidLaunched();

    if (!Core::Instance() || !DAVA::SystemTimer::Instance() || !RenderManager::Instance() || !CorePlatformWinStore::Instance() || !UIControlSystem::Instance() || !RenderSystem2D::Instance() || !VirtualCoordinatesSystem::Instance())
    {
        return;
    }

    Core::Instance()->SystemAppStarted();

    while (!isWindowClosed)
    {
        if (isWindowVisible)
        {
            CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
            DAVA::uint64 startTime = DAVA::SystemTimer::Instance()->AbsoluteMS();
            RenderManager::Instance()->Lock();
            CorePlatformWinStore::Instance()->SystemProcessFrame();
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
        else
        {
            CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
        }
    }
    RenderManager::Instance()->Release();

    ApplicationCore * appCore = Core::Instance()->GetApplicationCore();
    if (appCore && appCore->OnQuit())
    {
        exit(0);
    }
    else
    {
        willQuit = true;
    }
    Core::Instance()->SystemAppFinished();
    FrameworkWillTerminate();
    Core::Instance()->ReleaseSingletons();
}

// This method is called before the application exits.
void WinStoreFrame::Uninitialize()
{
}

void WinStoreFrame::OnActivated(_In_ CoreApplicationView^ /* applicationView */, _In_ IActivatedEventArgs^ /* args */)
{
    // Activate the application window, making it visible and enabling it to receive events.
    CoreWindow::GetForCurrentThread()->Activate();
    Core::Instance()->SetIsActive(true);
}

void WinStoreFrame::OnSuspending(_In_ Platform::Object^ /* sender */, _In_ SuspendingEventArgs^ args)
{
    isWindowVisible = false;
    Core::Instance()->SetIsActive(isWindowVisible);
}

void WinStoreFrame::OnResuming(_In_ Platform::Object^ /* sender */, _In_ Platform::Object^ /* args */)
{
    isWindowVisible = true;
    Core::Instance()->SetIsActive(isWindowVisible);
}

void WinStoreFrame::OnWindowActivationChanged(_In_ CoreWindow^ sender, _In_ WindowActivatedEventArgs^ args)
{
    if (nullptr != args)
    {
        return;
    }
    //TODO::makkis:: delete after
    CoreWindowActivationState state = args->WindowActivationState;
    switch (args->WindowActivationState)
    {
    case CoreWindowActivationState::CodeActivated:
        isWindowVisible = true;
        Core::Instance()->SetIsActive(isWindowVisible);
        break;
    case CoreWindowActivationState::Deactivated:
        isWindowVisible = false;
        Core::Instance()->SetIsActive(isWindowVisible);
        break;
    case CoreWindowActivationState::PointerActivated:
        break;
    }
}

void WinStoreFrame::OnWindowClosed(_In_ CoreWindow^ sender, _In_ CoreWindowEventArgs^ args)
{
    isWindowClosed = true;
}

void WinStoreFrame::OnVisibilityChanged(_In_ CoreWindow^ sender, _In_ VisibilityChangedEventArgs^ args)
{
    if (nullptr != args)
    {
        return;
    }
    isWindowVisible = args->Visible;
    Core::Instance()->SetIsActive(isWindowVisible);
}

// This method is called whenever the application window size changes.
// Most of this source copied from QtLayer::Resize()
void WinStoreFrame::OnWindowSizeChanged(_In_ CoreWindow^ sender, _In_ WindowSizeChangedEventArgs^ args)
{
    if (nullptr == args)
    {
        return;
    }
    //windows size change only here
    UpdateScreenSize(args->Size.Width, args->Size.Height);
    ReInitRender();
    ReInitCoordinatesSystem();
    UIScreenManager::Instance()->ScreenSizeChanged();
}

void WinStoreFrame::OnDpiChanged(_In_ DisplayInformation^ sender, _In_ Platform::Object^ args)
{
}

//TODO::makkis
void WinStoreFrame::OnOrientationChanged(_In_ DisplayInformation^ sender, _In_ Platform::Object^ args)
{
    if (nullptr == sender)
    {
        return;
    }
    displayOrientation = sender->CurrentOrientation;
    switch (displayOrientation)
    {
    //MSDN::0 No display orientation is specified.
    case Windows::Graphics::Display::DisplayOrientations::None:
        break;
    //MSDN::1 Specifies that the monitor is oriented in landscape mode where the width of the display viewing area is greater than the height.
    case Windows::Graphics::Display::DisplayOrientations::Landscape:
        break;
    //MSDN::2 Specifies that the monitor rotated 90 degrees in the clockwise direction to orient the display in portrait mode where the height of the display viewing area is greater than the width.
    case Windows::Graphics::Display::DisplayOrientations::Portrait:
        break;
    //MSDN::4 Specifies that the monitor rotated another 90 degrees in the clockwise direction (to equal 180 degrees) to orient the display in landscape mode where the width of the display viewing area is greater than the height. This landscape mode is flipped 180 degrees from the Landscape mode.
    case Windows::Graphics::Display::DisplayOrientations::LandscapeFlipped:
        break;
    //MSDN::8 Specifies that the monitor rotated another 90 degrees in the clockwise direction (to equal 270 degrees) to orient the display in portrait mode where the height of the display viewing area is greater than the width. This portrait mode is flipped 180 degrees from the Portrait mode.
    case Windows::Graphics::Display::DisplayOrientations::PortraitFlipped:
        break;
    }
}

void WinStoreFrame::OnDisplayContentsInvalidated(_In_ DisplayInformation^ sender, _In_ Platform::Object^ args)
{
}

void WinStoreFrame::OnStereoEnabledChanged(_In_ DisplayInformation^ sender, _In_ Platform::Object^ args)
{
}

/*
//TODO::makkis::https://social.msdn.microsoft.com/Forums/en-US/66fa24cd-5dc1-45c5-9976-ac5b2a6a4d59/display-resolution-api?forum=formobiledevicesru
Size WinStoreFrame::ScreenResolutionDetect()
{
    ViewManagement::ApplicationView^ view = ViewManagement::ApplicationView::GetForCurrentView();
    if (nullptr != view)
    {
        Windows::Foundation::Rect vis_bounds = view->VisibleBounds;
        bool is_lock = view->IsOnLockScreen;
        bool isful = view->IsFullScreenMode;
        int id = view->GetApplicationViewIdForWindow(CoreWindow::GetForCurrentThread());
        int id2 = view->Id;
        bool isCapture = view->IsScreenCaptureEnabled;
        int i = 0;
    }
//    ViewManagement::ProjectionManager^ projMgr = ViewManagement::ProjectionManager:://GetForCurrentView();
    
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
    return Size();
}
*/

void WinStoreFrame::InitInput()
{
    // detect mouse and touch
    // MSDN::This property always returns 1 if an instance of the Microsoft Visual Studio simulator is running.
    TouchCapabilities^ touchCapabilities = ref new TouchCapabilities();
    if (nullptr != touchCapabilities)
    {
        isTouchDetected = (1 == touchCapabilities->TouchPresent);
    }
    delete touchCapabilities;
    MouseCapabilities^ mouseCapabilities = ref new MouseCapabilities();
    if (nullptr != mouseCapabilities)
    {
        isMouseDetected = (1 == mouseCapabilities->MousePresent);
    }
    delete mouseCapabilities;
    // TODO::makkis temporary overhead
    ViewManagement::UIViewSettings^ settings = ViewManagement::UIViewSettings::GetForCurrentView();
    if (settings)
    {
        userInteractionMode = settings->UserInteractionMode;
    }
    if (ViewManagement::UserInteractionMode::Mouse == userInteractionMode)
    {
        //TODO::makkis
    }
    else // ViewManagement::UserInteractionMode::Touch == userInteractionMode
    {
        //TODO::makkis
    }
}

void WinStoreFrame::DAVATouchEvent(DAVA::UIEvent::eInputPhase phase, PointerPoint^ pointPtr)
{
    if (nullptr == pointPtr)
    {
        return;
    }
    Point pointerPosition = pointPtr->Position;
    //legacy
    Vector<DAVA::UIEvent> touches;
    int button = 0;
    if (isLeftButtonPressed)
        button = 1;
    else if (isRightButtonPressed)
        button = 2;
    else if (isMiddleButtonPressed)
        button = 3;
    bool isFind = false;
    for (Vector<DAVA::UIEvent>::iterator it = allTouches.begin(); it != allTouches.end(); ++it)
    {
        if (it->tid == button)
        {
            isFind = true;
            it->physPoint.x = pointerPosition.X;
            it->physPoint.y = pointerPosition.Y;
            it->phase = phase;
            break;
        }
    }
    if (!isFind)
    {
        UIEvent newTouch;
        newTouch.tid = button;
        newTouch.physPoint.x = pointerPosition.X;
        newTouch.physPoint.y = pointerPosition.Y;
        newTouch.phase = phase;
        allTouches.push_back(newTouch);
    }
    for (Vector<DAVA::UIEvent>::iterator it = allTouches.begin(); it != allTouches.end(); ++it)
    {
        touches.push_back(*it);
    }
    if (phase == UIEvent::PHASE_ENDED)
    {
        for (Vector<DAVA::UIEvent>::iterator it = allTouches.begin(); it != allTouches.end(); ++it)
        {
            if (it->tid == button)
            {
                allTouches.erase(it);
                break;
            }
        }
    }
    UIControlSystem::Instance()->OnInput(phase, touches, allTouches);
}

//TODO::makkis::https://social.msdn.microsoft.com/Forums/ru-RU/d4dca5cc-deb6-4c23-b1fc-ba66d94ab570/tryenterfullscreenmode-dont-work?forum=Win10SDKToolsIssues
void WinStoreFrame::SetFullScreen(bool isFullScreen)
{
    ViewManagement::ApplicationView^ view = ViewManagement::ApplicationView::GetForCurrentView();
    if (nullptr == view)
    {
        return;
    }
    bool isFull = view->IsFullScreenMode;
    if (isFull == isFullScreen)
    {
        return;
    }
    if (isFullScreen)
    {
        view->TryEnterFullScreenMode();
        view->PreferredLaunchWindowingMode = ViewManagement::ApplicationViewWindowingMode::FullScreen;
    }
    else
    {
        view->ExitFullScreenMode();
        view->PreferredLaunchWindowingMode = ViewManagement::ApplicationViewWindowingMode::PreferredLaunchViewSize;
    }
}

void WinStoreFrame::PreparationSizeScreen()
{
    //TODO::makkis::https://social.msdn.microsoft.com/Forums/en-US/66fa24cd-5dc1-45c5-9976-ac5b2a6a4d59/display-resolution-api?forum=formobiledevicesru
    //     Size screenSize = ScreenResolutionDetect();
    //     fullscreenMode.width = static_cast<int32>(screenSize.Width);
    //     fullscreenMode.height = static_cast<int32>(screenSize.Height);
    //     fullscreenMode.bpp = DISPLAY_MODE_DEFAULT_BITS_PER_PIXEL;
    //     fullscreenMode.refreshRate = DISPLAY_MODE_DEFAULT_DISPLAYFREQUENCY;
    KeyedArchive *options = CorePlatformWinStore::Instance()->GetOptions();
    if (options)
    {
        windowedMode.width = options->GetInt32("width", DISPLAY_MODE_DEFAULT_WIDTH);
        windowedMode.height = options->GetInt32("height", DISPLAY_MODE_DEFAULT_HEIGHT);
        windowedMode.bpp = options->GetInt32("bpp", DISPLAY_MODE_DEFAULT_BITS_PER_PIXEL);

        // get values from config in case if they are available
        fullscreenMode.width = options->GetInt32("fullscreen.width", fullscreenMode.width);
        fullscreenMode.height = options->GetInt32("fullscreen.height", fullscreenMode.height);
        fullscreenMode.bpp = windowedMode.bpp;
        //TODO::makkis::https://social.msdn.microsoft.com/Forums/en-US/66fa24cd-5dc1-45c5-9976-ac5b2a6a4d59/display-resolution-api?forum=formobiledevicesru
        //         fullscreenMode = CorePlatformWinStore::Instance()->FindBestMode(fullscreenMode);
        isFullscreen = (0 != options->GetInt32("fullscreen", 0));
    }
    Logger::FrameworkDebug("[PlatformWin32] best display fullscreen mode matched: %d x %d x %d refreshRate: %d", fullscreenMode.width, fullscreenMode.height, fullscreenMode.bpp, fullscreenMode.refreshRate);
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

void WinStoreFrame::UpdateScreenSize(float32 width, float32 height)
{
    windowWidth = width;
    windowHeight = height;

    exWindowWidth = static_cast<int32>(windowWidth * rawPixelInViewPixel);
    exWindowHeight = static_cast<int32>(windowHeight * rawPixelInViewPixel);
}

void WinStoreFrame::InitRender()
{
    RenderManager::Create(Core::RENDERER_OPENGL_ES_2_0);
    RenderManager::Instance()->Create(ownWindow.Get());
    // RenderManager::Instance()->ChangeDisplayMode(DisplayMode(exWindowWidth, exWindowHeight, DISPLAY_MODE_DEFAULT_BITS_PER_PIXEL, DISPLAY_MODE_DEFAULT_DISPLAYFREQUENCY), isFullscreen);
    RenderManager::Instance()->Init(exWindowWidth, exWindowHeight);
    RenderSystem2D::Instance()->Init();
}

void WinStoreFrame::ReInitRender()
{
    RenderManager::Instance()->Init(exWindowWidth, exWindowHeight);
    RenderSystem2D::Instance()->Init();
}

void WinStoreFrame::InitCoordinatesSystem()
{
    VirtualCoordinatesSystem::Instance()->SetInputScreenAreaSize(static_cast<int32>(windowWidth), static_cast<int32>(windowHeight));
    VirtualCoordinatesSystem::Instance()->SetPhysicalScreenSize(exWindowWidth, exWindowHeight);
    VirtualCoordinatesSystem::Instance()->EnableReloadResourceOnResize(true);
}

void WinStoreFrame::ReInitCoordinatesSystem()
{
    int32 intWidth = static_cast<int32>(windowWidth);
    int32 intHeight = static_cast<int32>(windowHeight);
    VirtualCoordinatesSystem::Instance()->SetInputScreenAreaSize(intWidth, intHeight);
    //if (1.0 != rawPixelInViewPixel)
    //{
        VirtualCoordinatesSystem::Instance()->UnregisterAllAvailableResourceSizes();
        VirtualCoordinatesSystem::Instance()->RegisterAvailableResourceSize(exWindowWidth, exWindowHeight, "Gfx");
    //}
    VirtualCoordinatesSystem::Instance()->SetPhysicalScreenSize(exWindowWidth, exWindowHeight);
    VirtualCoordinatesSystem::Instance()->SetVirtualScreenSize(intWidth, intHeight);
    VirtualCoordinatesSystem::Instance()->ScreenSizeChanged();
}

void WinStoreFrame::SetPreferredSize(const int32 &width, const int32 &height)
{
    ViewManagement::ApplicationView^ view = ViewManagement::ApplicationView::GetForCurrentView();
    if (nullptr != view)
    {
        // MSDN::This property only has an effect when the app is launched on a desktop device that is not in tablet mode.
        view->PreferredLaunchViewSize = Windows::Foundation::Size(static_cast<float32>(width), static_cast<float32>(height));
    }
}

void WinStoreFrame::SetTitleName()
{
    KeyedArchive *options = CorePlatformWinStore::Instance()->GetOptions();
    if (nullptr != options)
    {
        String lowTitle = options->GetString("title", "[set application title using core options property 'title']");
        Platform::String^ wideTitle = ref new Platform::String(StringToWString(lowTitle).c_str());
        ViewManagement::ApplicationView^ view = ViewManagement::ApplicationView::GetForCurrentView();
        if (nullptr != view)
        {
            view->Title = wideTitle;
        }
    }
}

DisplayOrientations WinStoreFrame::GetDisplayOrientation()
{
    return displayOrientation;
}

void WinStoreFrame::SetDisplayOrientations(Core::eScreenOrientation orientMode)
{
    displayOrientation = DisplayOrientations::None;
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
    DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();
    if (nullptr != currentDisplayInformation)
    {
        currentDisplayInformation->AutoRotationPreferences = displayOrientation;
    }
}

ViewManagement::ApplicationViewWindowingMode WinStoreFrame::GetScreenMode()
{
    if (isFullscreen)
    {
        return ViewManagement::ApplicationViewWindowingMode::FullScreen;
    }
    return ViewManagement::ApplicationViewWindowingMode::PreferredLaunchViewSize;
}

void WinStoreFrame::SetScreenMode(ViewManagement::ApplicationViewWindowingMode screenMode)
{
    if (ViewManagement::ApplicationViewWindowingMode::FullScreen == screenMode)
    {
        SetFullScreen(true);
    }
    else
    {
        SetFullScreen(false);
    }
    // TODO::makkis:: add auto
}

void WinStoreFrame::OnPointerPressed(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
{
    PointerPoint^ pointPtr = args->CurrentPoint;
    if (pointPtr)
    {
        PointerPointProperties^ pointProperties = pointPtr->Properties;
        //update state
        isLeftButtonPressed = pointProperties->IsLeftButtonPressed;
        isRightButtonPressed = pointProperties->IsRightButtonPressed;;
        isMiddleButtonPressed = pointProperties->IsMiddleButtonPressed;
        DAVATouchEvent(UIEvent::PHASE_BEGAN, pointPtr);
    }
}

void WinStoreFrame::OnPointerMoved(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
{
    switch (args->CurrentPoint->PointerDevice->PointerDeviceType)
    {
    case PointerDeviceType::Touch:
        DAVATouchEvent(UIEvent::PHASE_DRAG, args->CurrentPoint);
        break;
    case PointerDeviceType::Mouse:
        if (args->CurrentPoint->Properties->IsLeftButtonPressed)
        {
            DAVATouchEvent(UIEvent::PHASE_DRAG, args->CurrentPoint);
        }
        break;
    case PointerDeviceType::Pen:
        break;
    default:
        break;
    }
}

void WinStoreFrame::OnPointerReleased(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
{
    PointerPoint^ pointPtr = args->CurrentPoint;
    if (pointPtr)
    {
        DAVATouchEvent(UIEvent::PHASE_ENDED, args->CurrentPoint);
        PointerPointProperties^ pointProperties = pointPtr->Properties;
        //update state
        isLeftButtonPressed = pointProperties->IsLeftButtonPressed;
        isRightButtonPressed = pointProperties->IsRightButtonPressed;;
        isMiddleButtonPressed = pointProperties->IsMiddleButtonPressed;
    }
}

void WinStoreFrame::OnPointerExited(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
{
    if (!isMouseCursorShown)
    {
        ShowCursor();
        isMouseCursorShown = true;
    }
}

void WinStoreFrame::OnKeyDown(_In_ CoreWindow^ sender, _In_ KeyEventArgs^ args)
{
    auto window = CoreWindow::GetForCurrentThread();
    if (window)
    {
        CoreVirtualKeyStates menuStatus = window->GetKeyState(VirtualKey::Menu);
        CoreVirtualKeyStates tabStatus = window->GetKeyState(VirtualKey::Tab);
        // if CoreVirtualKeyStates::Down or CoreVirtualKeyStates::Locked
        bool isPressOrLock = static_cast<bool>((menuStatus & CoreVirtualKeyStates::Down) & (tabStatus & CoreVirtualKeyStates::Down));
        if (isPressOrLock)
        {
            __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
                //ShowWindow(hWnd, SW_MINIMIZE);
        }
    }

    Vector<DAVA::UIEvent> touches;
    DAVA::UIEvent ev;
    ev.keyChar = 0;
    ev.phase = DAVA::UIEvent::PHASE_KEYCHAR;
    ev.tapCount = 1;
    ev.tid = InputSystem::Instance()->GetKeyboard().GetDavaKeyForSystemKey(static_cast<int32>(args->VirtualKey));
    touches.push_back(ev);
    UIControlSystem::Instance()->OnInput(0, touches, allTouches);
    touches.pop_back();
    UIControlSystem::Instance()->OnInput(0, touches, allTouches);
    InputSystem::Instance()->GetKeyboard().OnSystemKeyPressed(static_cast<int32>(args->VirtualKey));
}

void WinStoreFrame::OnKeyUp(_In_ CoreWindow^ sender, _In_ KeyEventArgs^ args)
{
    InputSystem::Instance()->GetKeyboard().OnSystemKeyUnpressed(static_cast<int32>(args->VirtualKey));
}

void WinStoreFrame::OnWheel(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
{
    Vector<DAVA::UIEvent> touches;
    PointerPoint^ point = args->CurrentPoint;
    PointerPointProperties^ pointProperties = point->Properties;
    int32 wheelDelta = pointProperties->MouseWheelDelta;
    UIEvent newTouch;
    newTouch.tid = 0;
    newTouch.physPoint.x = 0;
    newTouch.physPoint.y = (int32)(wheelDelta) / (float32)(WHEEL_DELTA);
    newTouch.phase = UIEvent::PHASE_WHEEL;
    touches.push_back(newTouch);
    UIControlSystem::Instance()->OnInput(UIEvent::PHASE_WHEEL, touches, allTouches);
}

void WinStoreFrame::OnMouseMoved(_In_ MouseDevice^ mouseDevice, _In_ MouseEventArgs^ args)
{
}

void WinStoreFrame::ShowCursor()
{
    // Turn on mouse cursor.
    // This also disables relative mouse movement tracking.
    auto window = CoreWindow::GetForCurrentThread();
    if (window)
    {
        // Protect case where there isn't a window associated with the current thread.
        // This happens on initialization or when being called from a background thread.
        window->PointerCursor = ref new CoreCursor(CoreCursorType::Arrow, 0);
    }
}

void WinStoreFrame::HideCursor()
{
    // Turn mouse cursor off (hidden).
    // This enables relative mouse movement tracking.
    auto window = CoreWindow::GetForCurrentThread();
    if (window)
    {
        // Protect case where there isn't a window associated with the current thread.
        // This happens on initialization or when being called from a background thread.
        window->PointerCursor = nullptr;
    }
}

} // namespace DAVA

#endif // #if defined(__DAVAENGINE_WIN_UAP__)
