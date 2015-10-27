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

#ifndef __DAVAENGINE_DEFERREDEVENTS_H__
#define __DAVAENGINE_DEFERREDEVENTS_H__

#include "Base/Platform.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "FileSystem/Logger.h"
#include <functional>
#include "time.h"

namespace DAVA
{
using namespace ::Platform;
using namespace ::Windows::UI::Xaml;
using namespace ::Windows::UI::Xaml::Controls;
using namespace ::Windows::Foundation;

const int32 DEFERRED_INTERVAL_MSEC = 100;

// when app going in fullscreen or back
// we receive events with intermediate sizes
class DeferredScreenMetricEvents
{
public:
    using UpdateMetricCallback = std::function<void(bool isSizeUpdate, float32 widht, float32 height, bool isScaleUpdate, float32 scaleX, float32 scaleY)>;
    DeferredScreenMetricEvents(int32 interval, UpdateMetricCallback update);
    ~DeferredScreenMetricEvents();
    void UpdateSize(Object ^ sizeSender, SizeChangedEventArgs ^ sizeArgs);
    void UpdateScale(SwapChainPanel ^ scalePanel, Object ^ scaleArgs);

private:
    void DeferredTick();
    DispatcherTimer ^ timer;
    int32 interval = 0;

    bool isSizeUpdate = false;
    bool isScaleUpdate = false;
    float32 widht = 0.0f;
    float32 height = 0.0f;
    float32 scaleX = 0.0f;
    float32 scaleY = 0.0f;

    UpdateMetricCallback updateCallback;
};

DeferredScreenMetricEvents::DeferredScreenMetricEvents(int32 interval, UpdateMetricCallback update)
    : interval(interval)
    , updateCallback(update)
{
    timer = ref new DispatcherTimer();
    TimeSpan span;
    span.Duration = interval * 10000; // convert to 100ns ticks
    timer->Interval = span;
    auto tick = ref new EventHandler<Platform::Object ^>([this](Object ^ sender, Object ^ e) {
        DeferredTick();
    });
    timer->Tick += tick;
}

DeferredScreenMetricEvents::~DeferredScreenMetricEvents()
{
}

void DeferredScreenMetricEvents::UpdateSize(Object ^ sizeSender, SizeChangedEventArgs ^ sizeArgs)
{
    isSizeUpdate = true;
    if (timer->IsEnabled)
    {
        timer->Stop();
        timer->Start();
    }
    else
    {
        timer->Start();
    }
    widht = sizeArgs->NewSize.Width;
    height = sizeArgs->NewSize.Height;
}

void DeferredScreenMetricEvents::UpdateScale(SwapChainPanel ^ scalePanel, Object ^ scaleArgs)
{
    isScaleUpdate = true;
    if (timer->IsEnabled)
    {
        timer->Stop();
        timer->Start();
    }
    else
    {
        timer->Start();
    }
    scaleX = scalePanel->CompositionScaleX;
    scaleY = scalePanel->CompositionScaleY;
}

void DeferredScreenMetricEvents::DeferredTick()
{
    timer->Stop();
    updateCallback(isSizeUpdate, widht, height, isScaleUpdate, scaleX, scaleY);
    isSizeUpdate = false;
    isScaleUpdate = false;
    widht = height = scaleX = scaleY = 0.0f;
}

} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_DEFERREDEVENTS_H__
