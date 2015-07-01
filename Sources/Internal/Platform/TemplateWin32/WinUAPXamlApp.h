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

#include "Core/Core.h"
#include "Core/DisplayMode.h"

#include "UI/UIEvent.h"

namespace DAVA
{

ref class WinUAPXamlApp sealed : public ::Windows::UI::Xaml::Application
{
public:
    // Deleted and defaulted functions are not supported in WinRT classes
    WinUAPXamlApp();

    Windows::Graphics::Display::DisplayOrientations GetDisplayOrientation();
    Windows::UI::ViewManagement::ApplicationViewWindowingMode GetScreenMode();
    void SetScreenMode(Windows::UI::ViewManagement::ApplicationViewWindowingMode screenMode);

    Windows::UI::Core::CoreDispatcher^ Dispatcher();

    void AddUIElement(Windows::UI::Xaml::UIElement^ uiElement);
    void RemoveUIElement(Windows::UI::Xaml::UIElement^ uiElement);
    void PositionUIElement(Windows::UI::Xaml::UIElement^ uiElement, float32 x, float32 y);

protected:
    void OnLaunched(::Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ args) override;

private:
    void StartRenderLoop();
    void StopRenderLoop();
    void Run(Windows::Foundation::IAsyncAction^ action);

private:    // Event handlers
    // App state handlers
    void OnSuspending(::Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ args);
    void OnResuming(::Platform::Object^ sender, ::Platform::Object^ args);

    // Windows state change handlers
    void OnWindowActivationChanged(::Windows::UI::Core::CoreWindow^ sender, ::Windows::UI::Core::WindowActivatedEventArgs^ args);
    void OnWindowVisibilityChanged(::Windows::UI::Core::CoreWindow^ sender, ::Windows::UI::Core::VisibilityChangedEventArgs^ args);
    void OnWindowSizeChanged(::Windows::UI::Core::CoreWindow^ sender, ::Windows::UI::Core::WindowSizeChangedEventArgs^ args);

    // Mouse and touch handlers
    void OnPointerPressed(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ args);
    void OnPointerReleased(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ args);
    void OnPointerMoved(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ args);
    void OnPointerEntered(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ args);
    void OnPointerExited(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ args);
    void OnPointerWheel(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ args);
    void OnPointerCaptureLost(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ args);

    // Keyboard handlers
    void OnKeyDown(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args);
    void OnKeyUp(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args);

    void DAVATouchEvent(UIEvent::eInputPhase phase, Windows::UI::Input::PointerPoint^ pointerPoint);

private:
    void SetupEventHandlers();
    void SetupRenderLoopEventHandlers();
    void CreateBaseXamlUI();

    void SetTitleName();
    void SetDisplayOrientations(Core::eScreenOrientation orientation);

    void InitInput();

    void InitRender();
    void ReInitRender();

    void InitCoordinatesSystem();
    void ReInitCoordinatesSystem();

    void PrepareScreenSize();
    void UpdateScreenSize(float32 width, float32 height);
    void SetFullScreen(bool isFullScreenFlag);
    void SetPreferredSize(int32 width, int32 height);

    void ShowCursor();
    void HideCursor();

    template<typename F>
    Windows::Foundation::IAsyncAction^ RunOnUIThread(F fn);

    template<typename F>
    Windows::Foundation::IAsyncAction^ RunOnRenderThread(F fn);

private:
    Windows::UI::Core::CoreDispatcher^ coreDispatcher = nullptr;
    Windows::UI::Xaml::Controls::SwapChainPanel^ swapChainPanel = nullptr;
    Windows::UI::Xaml::Controls::Canvas^ canvas = nullptr;

    Windows::UI::Core::CoreIndependentInputSource^ renderLoopInput = nullptr;
    Windows::Foundation::IAsyncAction^ renderLoopWorker = nullptr;

    Vector<UIEvent> allTouches;

    bool isMouseDetected = false;
    bool isTouchDetected = false;

    bool isWindowVisible = true;
    bool isWindowClosed = false;
    bool isFullscreen = false;
    DisplayMode windowedMode = DisplayMode(DISPLAY_MODE_DEFAULT_WIDTH,
                                           DISPLAY_MODE_DEFAULT_HEIGHT,
                                           DISPLAY_MODE_DEFAULT_BITS_PER_PIXEL,
                                           DISPLAY_MODE_DEFAULT_DISPLAYFREQUENCY);
    DisplayMode currentMode = windowedMode;
    DisplayMode fullscreenMode = windowedMode;

    bool isMouseCursorShown = false;
    bool isRightButtonPressed = false;
    bool isLeftButtonPressed = false;
    bool isMiddleButtonPressed = false;

    float64 rawPixelInViewPixel = 1.0;
    float32 windowWidth = static_cast<float32>(DisplayMode::DISPLAY_MODE_DEFAULT_WIDTH);
    float32 windowHeight = static_cast<float32>(DisplayMode::DISPLAY_MODE_DEFAULT_HEIGHT);
    int32 integralWindowWidth = static_cast<int32>(windowWidth * rawPixelInViewPixel);
    int32 integralWindowHeight = static_cast<int32>(windowHeight * rawPixelInViewPixel);

    Windows::UI::ViewManagement::UserInteractionMode userInteractionMode = ::Windows::UI::ViewManagement::UserInteractionMode::Mouse;
    Windows::Graphics::Display::DisplayOrientations displayOrientation = ::Windows::Graphics::Display::DisplayOrientations::None;
};

//////////////////////////////////////////////////////////////////////////
inline Windows::UI::Core::CoreDispatcher^ WinUAPXamlApp::Dispatcher()
{
    return coreDispatcher;
}

template<typename F>
Windows::Foundation::IAsyncAction^ WinUAPXamlApp::RunOnUIThread(F fn)
{
    using namespace Windows::UI::Core;
    return coreDispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler(fn));
}

template<typename F>
Windows::Foundation::IAsyncAction^ WinUAPXamlApp::RunOnRenderThread(F fn)
{
    using namespace Windows::UI::Core;
    return renderLoopInput->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler(fn));
}

}   // namespace DAVA

#endif  // __DAVAENGINE_WIN_UAP__
#endif  // __DAVAENGINE_WINUAPFRAME_H__
