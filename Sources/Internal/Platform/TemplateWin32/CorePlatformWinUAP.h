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


#ifndef __DAVAENGINE_CORE_PLATFORM_WINSTORE_H__
#define __DAVAENGINE_CORE_PLATFORM_WINSTORE_H__

#include "Base/Platform.h"
#if defined(__DAVAENGINE_WIN_UAP__)

#include "Core\Core.h"
#include "Base\BaseTypes.h"
#include <agile.h>

namespace DAVA
{

ref class WinStoreFrame sealed : public Windows::ApplicationModel::Core::IFrameworkView
{
public:
    WinStoreFrame();

    // IFrameworkView Methods
    virtual void Initialize(_In_ Windows::ApplicationModel::Core::CoreApplicationView^ applicationView);
    virtual void SetWindow(_In_ Windows::UI::Core::CoreWindow^ window);
    virtual void Load(_In_ Platform::String^ entryPoint);
    virtual void Run();
    virtual void Uninitialize();

protected:
    
    // Application lifecycle event handlers.
    void OnActivated(_In_ Windows::ApplicationModel::Core::CoreApplicationView^ applicationView, _In_ Windows::ApplicationModel::Activation::IActivatedEventArgs^ args);
    void OnSuspending(_In_ Platform::Object^ sender, _In_ Windows::ApplicationModel::SuspendingEventArgs^ args);
    void OnResuming(_In_ Platform::Object^ sender, _In_ Platform::Object^ args);
    // Window event handlers.
    void OnWindowActivationChanged(_In_ Windows::UI::Core::CoreWindow^ sender, _In_ Windows::UI::Core::WindowActivatedEventArgs^ args);
    void OnWindowClosed(_In_ Windows::UI::Core::CoreWindow^ sender, _In_ Windows::UI::Core::CoreWindowEventArgs^ args
        );
    // MSDN::Is fired when the window visibility is changed.
    void OnVisibilityChanged(_In_ Windows::UI::Core::CoreWindow^ sender, _In_ Windows::UI::Core::VisibilityChangedEventArgs^ args);
    // MSDN::Is fired when the window size is changed.
    void OnWindowSizeChanged(_In_ Windows::UI::Core::CoreWindow^ sender, _In_ Windows::UI::Core::WindowSizeChangedEventArgs^ args);
    // DisplayInformation event handlers. 
    // MSDN::Monitors and controls physical display information. The class provides events to allow clients to monitor for changes in the display.
    // MSDN::Occurs when the LogicalDpi property changes because the pixels per inch (PPI) of the display changes.
    void OnDpiChanged(_In_ Windows::Graphics::Display::DisplayInformation^ sender, _In_ Platform::Object^ args);
    // MSDN::Occurs when either the CurrentOrientation or NativeOrientation property changes because of a mode change or a monitor change.
    void OnOrientationChanged(_In_ Windows::Graphics::Display::DisplayInformation^ sender, _In_ Platform::Object^ args);
    // MSDN::Occurs when the display requires redrawing.
    void OnDisplayContentsInvalidated(_In_ Windows::Graphics::Display::DisplayInformation^ sender, _In_ Platform::Object^ args);
    // MSDN::Occurs when the StereoEnabled property changes because support for stereoscopic 3D changes.
    void OnStereoEnabledChanged(_In_ Windows::Graphics::Display::DisplayInformation^ sender, _In_ Platform::Object^ args);
    // CoreWindows events
    // MSDN::Occurs when a mouse button is clicked, or a touch or pen contact is detected, within the bounding rectangle of the Windows Store app.
    void OnPointerPressed(_In_ Windows::UI::Core::CoreWindow^ sender, _In_ Windows::UI::Core::PointerEventArgs^ args);
    // MSDN::Occurs when a pointer moves within the bounding box of the Windows Store app.
    void OnPointerMoved(_In_ Windows::UI::Core::CoreWindow^ sender, _In_ Windows::UI::Core::PointerEventArgs^ args);
    // MSDN::Occurs when a mouse button is released, or a touch or pen contact is lifted, within the bounding rectangle of the Windows Store app.
    void OnPointerReleased(_In_ Windows::UI::Core::CoreWindow^ sender, _In_ Windows::UI::Core::PointerEventArgs^ args);
    // MSDN::Occurs when the pointer moves outside the bounding box of the Windows Store app.
    void OnPointerExited(_In_ Windows::UI::Core::CoreWindow^ sender, _In_ Windows::UI::Core::PointerEventArgs^ args);
    // MSDN::Is fired when a non-system key is pressed.
    void OnKeyDown(_In_ Windows::UI::Core::CoreWindow^ sender, _In_ Windows::UI::Core::KeyEventArgs^ args);
    // MSDN::Is fired when a non-system key is released after a press.
    void OnKeyUp(_In_ Windows::UI::Core::CoreWindow^ sender, _In_ Windows::UI::Core::KeyEventArgs^ args);
    // MSDN::Occurs when the delta value of a pointer wheel changes.
    void OnWheel(_In_ Windows::UI::Core::CoreWindow^ sender, _In_ Windows::UI::Core::PointerEventArgs^ args);
    // MouseDevice class. Supports the ability to identify and track connected mouse devices.
    // MSDN::Occurs when the mouse pointer is moved.
    void OnMouseMoved(_In_ Windows::Devices::Input::MouseDevice^ mouseDevice, _In_ Windows::Devices::Input::MouseEventArgs^ args);

private:
    void ShowCursor();
    void HideCursor();
    void UpdateScreenSize(float32 width, float32 height);
    // TODO::makkis:: wait response from Microsoft
    //Windows::Foundation::Size ScreenResolutionDetect();
    void InitInput();
    void InitRender();
    void InitCoordinatesSystem();
    void PreparationSizeScreen();
    void ReInitRender();
    void ReInitCoordinatesSystem();

    void DAVATouchEvent(DAVA::UIEvent::eInputPhase phase, Windows::UI::Input::PointerPoint^ pointPtr);
    void SetFullScreen(bool isFullScreenFlag);
    void SetPreferredSize(const int32 &width, const int32 &height);
    void SetTitleName();
    void SetDisplayOrientations(Core::eScreenOrientation orientMode);

public:
    Windows::Graphics::Display::DisplayOrientations GetDisplayOrientation();
    Windows::UI::ViewManagement::ApplicationViewWindowingMode GetScreenMode();
    void SetScreenMode(Windows::UI::ViewManagement::ApplicationViewWindowingMode screenMode);

private:
    Platform::Agile<Windows::UI::Core::CoreWindow> ownWindow;
    Vector<DAVA::UIEvent> allTouches;

    // static properties
    bool isMouseDetected;
    bool isTouchDetected;
    // dynamic properties
    Windows::UI::ViewManagement::UserInteractionMode userInteractionMode;
    Windows::Graphics::Display::DisplayOrientations displayOrientation;
    bool willQuit;
    volatile bool isWindowClosed;
    volatile bool isWindowVisible;
    bool isFullscreen;
    // display's properties
    float32 windowWidth;
    float32 windowHeight;
    float64 rawPixelInViewPixel;
    int32 exWindowWidth;
    int32 exWindowHeight;
    // input's properties
    bool isMouseCursorShown;
    bool isRightButtonPressed;
    bool isLeftButtonPressed;
    bool isMiddleButtonPressed;

    DisplayMode currentMode;
    DisplayMode fullscreenMode;
    DisplayMode windowedMode;
    RECT windowPositionBeforeFullscreen;

};

class CorePlatformWinStore : public Core
{
public:
    CorePlatformWinStore();
    virtual ~CorePlatformWinStore();

    CorePlatformWinStore(const CorePlatformWinStore&) = delete;
    CorePlatformWinStore& operator=(const CorePlatformWinStore&) = delete;

    void Run();
    void InitArgs(); // if need
    eScreenMode GetScreenMode() override;
    void SwitchScreenToMode(eScreenMode screenMode) override;
    //void GetAvailableDisplayModes(List<DisplayMode> & availableModes) override;
    // TODO::makkis:: wait response from Microsoft
    //DisplayMode FindBestMode(const DisplayMode & requestedMode) override;
    // TODO::makkis:: wait response from Microsoft
    void Quit() override;
    void SetIcon(int32 iconId) override;
    eScreenOrientation GetScreenOrientation() override;
    uint32 GetScreenDPI() override;
    void GoBackground(bool isLock) override;
    void GoForeground() override;

private:
    Platform::Agile<WinStoreFrame> frameWinUAP;
};

ref class WinStoreApplicationSource : Windows::ApplicationModel::Core::IFrameworkViewSource
{
public:
    virtual Windows::ApplicationModel::Core::IFrameworkView^ CreateView();
    Windows::ApplicationModel::Core::IFrameworkView^ GetView() { return frame.Get(); }
private:
    Platform::Agile<Windows::ApplicationModel::Core::IFrameworkView> frame;
};

} // namespace DAVA

#endif // #if defined(__DAVAENGINE_WIN_UAP__)
#endif // __DAVAENGINE_CORE_PLATFORM_WINSTORE_H__