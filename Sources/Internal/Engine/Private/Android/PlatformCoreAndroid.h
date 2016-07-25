#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_ANDROID__)

#include "Engine/Private/EnginePrivateFwd.h"

namespace DAVA
{
namespace Private
{
class PlatformCore final
{
public:
    PlatformCore(EngineBackend* ebackend);
    ~PlatformCore();

    PlatformCore(const PlatformCore&) = delete;
    PlatformCore& operator=(const PlatformCore&) = delete;

    NativeService* GetNativeService() const;

    void Init();
    void Run();
    void Quit();

private:
    WindowBackend* CreateNativeWindow(Window* w, float32 width, float32 height);

private:
    EngineBackend* engineBackend = nullptr;
    std::unique_ptr<NativeService> nativeService;
};

inline NativeService* PlatformCore::GetNativeService() const
{
    return nativeService.get();
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_ANDROID__
#endif // __DAVAENGINE_COREV2__
