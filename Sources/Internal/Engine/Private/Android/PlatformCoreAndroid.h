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
    void GameThread();

    WindowBackend* OnCreate();
    void OnStart();
    void OnResume();
    void OnPause();
    void OnStop();
    void OnDestroy();

private:
    EngineBackend* engineBackend = nullptr;
    MainDispatcher* dispatcher = nullptr;
    AndroidBridge* bridge = nullptr;
    std::unique_ptr<NativeService> nativeService;

    bool quitGameThread = false;

    // Friends
    friend struct AndroidBridge;
};

inline NativeService* PlatformCore::GetNativeService() const
{
    return nativeService.get();
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_ANDROID__
#endif // __DAVAENGINE_COREV2__
