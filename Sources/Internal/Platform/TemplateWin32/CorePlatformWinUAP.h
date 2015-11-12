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

#ifndef __DAVAENGINE_COREPLATFORMWINUAP_H__
#define __DAVAENGINE_COREPLATFORMWINUAP_H__

#include "Base/Platform.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Base/BaseTypes.h"
#include "Core/Core.h"
#include "Platform/DeviceInfo.h"
#include "Input/InputSystem.h"

namespace DAVA
{

ref class WinUAPXamlApp;

class CorePlatformWinUAP : public Core
{
public:
    CorePlatformWinUAP() = default;
    virtual ~CorePlatformWinUAP() = default;

    CorePlatformWinUAP(const CorePlatformWinUAP&) = delete;
    CorePlatformWinUAP& operator = (const CorePlatformWinUAP&) = delete;

    void InitArgs();
    void Run();
    void Quit() override;

    eScreenMode GetScreenMode() override;
    bool SetScreenMode(eScreenMode screenMode) override;
    DisplayMode GetCurrentDisplayMode() override;

    void SetScreenScaleMultiplier(float32 multiplier) override;

    bool GetCursorVisibility();
    InputSystem::eMouseCaptureMode GetMouseCaptureMode();
    bool SetMouseCaptureMode(InputSystem::eMouseCaptureMode mode);

    // Win10 specific member functions

    // Get pointer to underlying XAML application object
    WinUAPXamlApp^ XamlApplication() DAVA_NOEXCEPT;

    // Check whether current thread is UI thread
    bool IsUIThread() const;

    // Run specified function on UI thread
    template<typename F>
    void RunOnUIThread(F&& fn);
    // Run specified function on UI thread and block calling thread until function finishes
    template<typename F>
    void RunOnUIThreadBlocked(F&& fn);

    // Run specified function on main thread
    template<typename F>
    void RunOnMainThread(F&& fn);
    // Run specified function on main thread and block calling thread until function finishes
    template<typename F>
    void RunOnMainThreadBlocked(F&& fn);

private:
    void RunOnUIThread(std::function<void()>&& fn, bool blocked);
    void RunOnMainThread(std::function<void()>&& fn, bool blocked);

private:
    WinUAPXamlApp^ xamlApp = nullptr;
};

//////////////////////////////////////////////////////////////////////////
inline WinUAPXamlApp^ CorePlatformWinUAP::XamlApplication() DAVA_NOEXCEPT
{
    return xamlApp;
}

template<typename F>
void CorePlatformWinUAP::RunOnUIThread(F&& fn)
{
    if (IsUIThread())
    {
        fn();
    }
    else
    {
        RunOnUIThread(std::function<void()>(std::forward<F>(fn)), false);
    }
}

template<typename F>
void CorePlatformWinUAP::RunOnUIThreadBlocked(F&& fn)
{
    if (IsUIThread())
    {
        fn();
    }
    else
    {
        RunOnUIThread(std::function<void()>(std::forward<F>(fn)), true);
    }
}

template<typename F>
void CorePlatformWinUAP::RunOnMainThread(F&& fn)
{
    RunOnMainThread(std::function<void()>(std::forward<F>(fn)), false);
}

template<typename F>
void CorePlatformWinUAP::RunOnMainThreadBlocked(F&& fn)
{
    RunOnMainThread(std::function<void()>(std::forward<F>(fn)), true);
}

}   // namespace DAVA

#endif  // __DAVAENGINE_WIN_UAP__
#endif  // __DAVAENGINE_COREPLATFORMWINUAP_H__
