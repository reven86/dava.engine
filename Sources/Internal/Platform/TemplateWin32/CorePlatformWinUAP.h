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
    CorePlatformWinUAP& operator=(const CorePlatformWinUAP&) = delete;

    void InitArgs();
    void Run();
    void Quit() override;

    eScreenMode GetScreenMode() override;
    bool SetScreenMode(eScreenMode screenMode) override;

    void SetWindowMinimumSize(float32 width, float32 height) override;
    Vector2 GetWindowMinimumSize() const override;

    // Win10 specific member functions

    // Get pointer to underlying XAML application object
    WinUAPXamlApp ^ XamlApplication() DAVA_NOEXCEPT;

    // Check whether current thread is UI thread
    bool IsUIThread() const;

    // Run specified function on UI thread
    template <typename F>
    void RunOnUIThread(F&& fn);
    // Run specified function on UI thread and block calling thread until function finishes
    template <typename F>
    void RunOnUIThreadBlocked(F&& fn);

    // Run specified function on main thread
    template <typename F>
    void RunOnMainThread(F&& fn);
    // Run specified function on main thread and block calling thread until function finishes
    template <typename F>
    void RunOnMainThreadBlocked(F&& fn);

private:
    void RunOnUIThread(std::function<void()>&& fn, bool blocked);
    void RunOnMainThread(std::function<void()>&& fn, bool blocked);

private:
    WinUAPXamlApp ^ xamlApp = nullptr;
};

//////////////////////////////////////////////////////////////////////////
inline WinUAPXamlApp ^ CorePlatformWinUAP::XamlApplication() DAVA_NOEXCEPT
{
    return xamlApp;
}

template <typename F>
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

template <typename F>
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

template <typename F>
void CorePlatformWinUAP::RunOnMainThread(F&& fn)
{
    RunOnMainThread(std::function<void()>(std::forward<F>(fn)), false);
}

template <typename F>
void CorePlatformWinUAP::RunOnMainThreadBlocked(F&& fn)
{
    RunOnMainThread(std::function<void()>(std::forward<F>(fn)), true);
}

} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREPLATFORMWINUAP_H__
