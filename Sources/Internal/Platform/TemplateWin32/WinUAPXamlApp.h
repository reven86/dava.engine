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

#ifndef __DAVAENGINE_WINUAPFRAME_H__
#define __DAVAENGINE_WINUAPFRAME_H__

#include "Base/Platform.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include <agile.h>
#include <concrt.h>

#include "Core/Core.h"
#include "Core/DisplayMode.h"

#include "UI/UIEvent.h"

namespace DAVA
{

class CorePlatformWinUAP;
class DispatcherWinUAP;

/************************************************************************
 Class WinUAPXamlApp represents WinRT XAML application with embedded framework's render loop
 On startup application creates minimal neccesary infrastructure to allow coexistance of
 XAML native controls and DirectX and OpenGL (through ANGLE).
 Application makes explicit use of two threads:
    - UI thread, created by system, where all interaction with UI and XAML controls must be done
    - main thread, created by WinUAPXamlApp instance, where framework lives
 To run code on UI thread you should use CorePlatformWinUAP::RunOnUIThread or CorePlatformWinUAP::RunOnUIThreadBlocked
 To run code on main thread you should use CorePlatformWinUAP::RunOnMainThread
************************************************************************/
ref class WinUAPXamlApp sealed : public ::Windows::UI::Xaml::Application
{
public:
    // Deleted and defaulted functions are not supported in WinRT classes
    WinUAPXamlApp();
    virtual ~WinUAPXamlApp();

    Windows::Graphics::Display::DisplayOrientations GetDisplayOrientation();
    Windows::UI::ViewManagement::ApplicationViewWindowingMode GetScreenMode();
    void SetScreenMode(Windows::UI::ViewManagement::ApplicationViewWindowingMode screenMode);
    Windows::Foundation::Size GetCurrentScreenSize();
    void SetCursorPinning(bool isPinning);
    void SetCursorVisible(bool isVisible);

    Windows::UI::Core::CoreDispatcher^ UIThreadDispatcher();

internal:   // Only internal methods of ref class can return pointers to non-ref objects
    DispatcherWinUAP* MainThreadDispatcher();

public:
    void SetQuitFlag();

    void AddUIElement(Windows::UI::Xaml::UIElement^ uiElement);
    void RemoveUIElement(Windows::UI::Xaml::UIElement^ uiElement);
    void PositionUIElement(Windows::UI::Xaml::UIElement^ uiElement, float32 x, float32 y);
    void SetTextBoxCustomStyle(Windows::UI::Xaml::Controls::TextBox^ textBox);
    void SetPasswordBoxCustomStyle(Windows::UI::Xaml::Controls::PasswordBox^ passwordBox);
    void UnfocusUIElement();

protected:
    void OnLaunched(::Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ args) override;

private:
    void Run();

private:    // Event handlers
    // App state handlers
    void OnSuspending(::Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ args);
    void OnResuming(::Platform::Object^ sender, ::Platform::Object^ args);

    // Windows state change handlers
    void OnWindowActivationChanged(::Windows::UI::Core::CoreWindow^ sender, ::Windows::UI::Core::WindowActivatedEventArgs^ args);
    void OnWindowVisibilityChanged(::Windows::UI::Core::CoreWindow^ sender, ::Windows::UI::Core::VisibilityChangedEventArgs^ args);
    
    // Swap chain panel state change handlers
    void OnSwapChainPanelSizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e);
    void OnSwapChainPanelScaleChanged(Windows::UI::Xaml::Controls::SwapChainPanel^ panel, Platform::Object^ args);

    // Mouse and touch handlers
    void OnPointerPressed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);
    void OnPointerReleased(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);
    void OnPointerMoved(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);
    void OnPointerEntered(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);
    void OnPointerExited(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);
    void OnPointerWheel(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);
    void OnHardwareBackButtonPressed(Platform::Object^ sender, Windows::Phone::UI::Input::BackPressedEventArgs ^args);

    // Keyboard handlers
    void OnKeyDown(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args);
    void OnKeyUp(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args);
    void OnMouseMoved(Windows::Devices::Input::MouseDevice^ mouseDevice, Windows::Devices::Input::MouseEventArgs^ args);

    void DAVATouchEvent(UIEvent::eInputPhase phase, float32 x, float32 y, int32 id);

private:
    void SetupEventHandlers();
    void CreateBaseXamlUI();

    void SetTitleName();
    void SetDisplayOrientations();

    void ResetRender();

    void InitCoordinatesSystem();
    void ReInitCoordinatesSystem();

    void PrepareScreenSize();
    void UpdateScreenSize(int32 width, int32 height);
    void UpdateScreenScale(float32 scaleX, float32 scaleY);
    void SetFullScreen(bool isFullScreenFlag);
    // in units of effective (view) pixels
    void SetPreferredSize(float32 width, float32 height);
    void HideAsyncTaskBar();
    
private:
    Concurrency::critical_section criticalSection;
    CorePlatformWinUAP* core;
    Windows::UI::Core::CoreDispatcher^ uiThreadDispatcher = nullptr;
    std::unique_ptr<DispatcherWinUAP> dispatcher = nullptr;

    Windows::UI::Xaml::Controls::SwapChainPanel^ swapChainPanel = nullptr;
    Windows::UI::Xaml::Controls::Canvas^ canvas = nullptr;
    Windows::UI::Xaml::Controls::Button^ controlThatTakesFocus = nullptr;
    Windows::UI::Xaml::Style^ customTextBoxStyle = nullptr;
    Windows::UI::Xaml::Style^ customPasswordBoxStyle = nullptr;

    Windows::Foundation::IAsyncAction^ renderLoopWorker = nullptr;

    volatile bool quitFlag = false;

    Vector<UIEvent> allTouches;

    bool isMouseDetected = false;
    bool isTouchDetected = false;
    bool isPhoneApiDetected = false;

    bool isWindowVisible = true;
    bool isWindowClosed = false;
    bool isFullscreen = false;
    bool isRenderCreated = false;
    DisplayMode windowedMode = DisplayMode(DisplayMode::DEFAULT_WIDTH,
                                           DisplayMode::DEFAULT_HEIGHT,
                                           DisplayMode::DEFAULT_BITS_PER_PIXEL,
                                           DisplayMode::DEFAULT_DISPLAYFREQUENCY);
    DisplayMode currentMode = windowedMode;
    DisplayMode fullscreenMode = windowedMode;

    bool isMouseCursorShown = false;
    bool isCursorPinning = false;
    bool isRightButtonPressed = false;
    bool isLeftButtonPressed = false;
    bool isMiddleButtonPressed = false;

    float32 swapChainScaleX = 1.f;
    float32 swapChainScaleY = 1.f;
    int32 swapChainWidth = DisplayMode::DEFAULT_WIDTH;
    int32 swapChainHeight = DisplayMode::DEFAULT_HEIGHT;
    int32 physicalWidth = static_cast<int32>(swapChainWidth * swapChainScaleX);
    int32 physicalHeight = static_cast<int32>(swapChainHeight * swapChainScaleY);

    Windows::Graphics::Display::DisplayOrientations displayOrientation = ::Windows::Graphics::Display::DisplayOrientations::None;

private:
    // Hardcoded styles for TextBox and PasswordBox to apply features:
    //  - transparent background in focus state
    //  - removed 'X' button
    static const wchar_t xamlTextBoxStyles[];
};

//////////////////////////////////////////////////////////////////////////
inline Windows::UI::Core::CoreDispatcher^ WinUAPXamlApp::UIThreadDispatcher()
{
    return uiThreadDispatcher;
}

inline DispatcherWinUAP* WinUAPXamlApp::MainThreadDispatcher()
{
    return dispatcher.get();
}

inline void WinUAPXamlApp::SetQuitFlag()
{
    quitFlag = true;
}

}   // namespace DAVA

#endif  // __DAVAENGINE_WIN_UAP__
#endif  // __DAVAENGINE_WINUAPFRAME_H__
