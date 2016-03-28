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

// clang-format off
#ifndef __DAVAENGINE_DEFERREDEVENTS_H__
#define __DAVAENGINE_DEFERREDEVENTS_H__

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_WIN_UAP__)

namespace DAVA
{

// when app going in fullscreen or back
// we receive events with intermediate sizes
class DeferredScreenMetricEvents
{
    static const int32 DEFERRED_INTERVAL_MSEC = 100;

public:
    using UpdateMetricCallback = std::function<void(float32 width, float32 height, float32 scaleX, float32 scaleY)>;

    DeferredScreenMetricEvents(bool isPhoneApi, UpdateMetricCallback callback, int32 intervalMs = DEFERRED_INTERVAL_MSEC);

    void SetWindowMinimumSize(float32 minWidth, float32 minHeight);
    Vector2 GetWindowMinimumSize() const;

    void CoreWindowSizeChanged(Windows::UI::Core::CoreWindow^ coreWindow, Windows::UI::Core::WindowSizeChangedEventArgs^ args);
    void SwapChainPanelSizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ args);
    void SwapChainPanelCompositionScaleChanged(Windows::UI::Xaml::Controls::SwapChainPanel^ swapChain, Platform::Object^);

private:
    void DeferredTick();

    Windows::UI::Xaml::DispatcherTimer^ timer = nullptr;
    bool isPhoneApiDetected = false;

    float32 width = 0.0f;
    float32 height = 0.0f;
    float32 scaleX = 0.0f;
    float32 scaleY = 0.0f;

    bool lockUpdate = false;
    float32 minWindowWidth = 0.0f;
    float32 minWindowHeight = 0.0f;

    UpdateMetricCallback updateCallback;
};

inline DeferredScreenMetricEvents::DeferredScreenMetricEvents(bool isPhoneApi, UpdateMetricCallback callback, int32 intervalMs)
    : timer(ref new Windows::UI::Xaml::DispatcherTimer)
    , isPhoneApiDetected(isPhoneApi)
    , updateCallback(callback)
{
    Windows::Foundation::TimeSpan span;
    span.Duration = intervalMs * 10000; // convert to 100ns ticks
    timer->Interval = span;

    auto tick = ref new Windows::Foundation::EventHandler<Platform::Object^>([this](Platform::Object^, Platform::Object^) {
        DeferredTick();
    });
    timer->Tick += tick;
}

inline void DeferredScreenMetricEvents::SetWindowMinimumSize(float32 minWidth, float32 minHeight)
{
    minWindowWidth = minWidth;
    minWindowHeight = minHeight;
}

inline Vector2 DeferredScreenMetricEvents::GetWindowMinimumSize() const
{
    return Vector2(minWindowWidth, minWindowHeight);
}

inline void DeferredScreenMetricEvents::CoreWindowSizeChanged(Windows::UI::Core::CoreWindow^ coreWindow, Windows::UI::Core::WindowSizeChangedEventArgs^ args)
{
    if (!isPhoneApiDetected)
    {
        lockUpdate = true;
        Windows::UI::Xaml::Controls::SwapChainPanel^ sw = dynamic_cast<Windows::UI::Xaml::Controls::SwapChainPanel^>(Windows::UI::Xaml::Window::Current->Content);
        DVASSERT(sw);
        scaleX = sw->CompositionScaleX;
        scaleY = sw->CompositionScaleY;
        width = static_cast<float32>(sw->ActualWidth);
        height = static_cast<float32>(sw->ActualHeight);
        timer->Start();
    }
}

inline void DeferredScreenMetricEvents::SwapChainPanelSizeChanged(Platform::Object^ swapChain, Windows::UI::Xaml::SizeChangedEventArgs^ args)
{
    Windows::UI::Xaml::Controls::SwapChainPanel^ sw = dynamic_cast<Windows::UI::Xaml::Controls::SwapChainPanel^>(swapChain);
    DVASSERT(sw);
    scaleX = sw->CompositionScaleX;
    scaleY = sw->CompositionScaleY;
    width = args->NewSize.Width;
    height = args->NewSize.Height;
    timer->Start();
}

inline void DeferredScreenMetricEvents::SwapChainPanelCompositionScaleChanged(Windows::UI::Xaml::Controls::SwapChainPanel^ swapChain, Platform::Object^)
{
    width = static_cast<float32>(swapChain->ActualWidth);
    height = static_cast<float32>(swapChain->ActualHeight);
    scaleX = swapChain->CompositionScaleX;
    scaleY = swapChain->CompositionScaleY;
    timer->Start();
}

inline void DeferredScreenMetricEvents::DeferredTick()
{
    Windows::Foundation::Rect windowRect = Windows::UI::Xaml::Window::Current->CoreWindow->Bounds;
    float32 windowScale = static_cast<float32>(Windows::Graphics::Display::DisplayInformation::GetForCurrentView()->RawPixelsPerViewPixel);
    DVASSERT(windowScale);
    float32 w = windowRect.Width;
    float32 h = windowRect.Height;
    float32 trueMinWidth = minWindowWidth / windowScale;
    float32 trueMinHeight = minWindowHeight / windowScale;
    DVASSERT(scaleX * scaleY);

    bool trackMinSize = trueMinWidth > 0.0f && trueMinHeight > 0.0f;
    if (!isPhoneApiDetected && trackMinSize && (w < trueMinWidth || h < trueMinHeight))
    {
        w = std::max(w, trueMinWidth);
        h = std::max(h, trueMinHeight);
        Windows::Foundation::Size size(w, h);
        auto currentView = Windows::UI::ViewManagement::ApplicationView::GetForCurrentView();

        bool success = currentView->TryResizeView(size);
        if (!success)
        {
            size.Width = trueMinWidth;
            size.Height = trueMinHeight;
            if (!currentView->TryResizeView(size))
            {
                Logger::FrameworkDebug("[DeferredScreenMetricEvents::DeferredTick]: Unable to resize window to minimum size");
            }
        }
    }
    else
    {
        lockUpdate = false;
    }

    if (!lockUpdate)
    {
        timer->Stop();
        updateCallback(width, height, scaleX, scaleY);
    }
}

} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_DEFERREDEVENTS_H__
