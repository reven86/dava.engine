#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_WIN32__)

#include "Engine/Private/EngineFwd.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace DAVA
{
namespace Private
{

class CoreWin32 final
{
public:
    CoreWin32(EngineBackend* e);
    ~CoreWin32();

    CoreWin32(const CoreWin32&) = delete;
    CoreWin32& operator=(const CoreWin32&) = delete;

    static HINSTANCE Win32AppInstance();

    NativeService* GetNativeService() const;

    void Init();
    void Run();
    void Quit();

private:
    WindowWin32* CreateNativeWindow(Window* w, float32 width, float32 height);

private:
    EngineBackend* engineBackend = nullptr;
    std::unique_ptr<NativeService> nativeService;

    static HINSTANCE hinstance;
};

inline HINSTANCE CoreWin32::Win32AppInstance()
{
    return hinstance;
}

inline NativeService* CoreWin32::GetNativeService() const
{
    return nativeService.get();
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN32__
#endif // __DAVAENGINE_COREV2__
