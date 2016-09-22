#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_WIN32__)

#include "Engine/Private/EnginePrivateFwd.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace DAVA
{
namespace Private
{
class PlatformCore final
{
public:
    PlatformCore(EngineBackend* engineBackend);
    ~PlatformCore();

    PlatformCore(const PlatformCore&) = delete;
    PlatformCore& operator=(const PlatformCore&) = delete;

    static HINSTANCE Win32AppInstance();

    NativeService* GetNativeService() const;

    void Init();
    void Run();
    void PrepareToQuit();
    void Quit();

private:
    EngineBackend& engineBackend;
    std::unique_ptr<NativeService> nativeService;

    static HINSTANCE hinstance;
};

inline HINSTANCE PlatformCore::Win32AppInstance()
{
    return hinstance;
}

inline NativeService* PlatformCore::GetNativeService() const
{
    return nativeService.get();
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN32__
#endif // __DAVAENGINE_COREV2__
