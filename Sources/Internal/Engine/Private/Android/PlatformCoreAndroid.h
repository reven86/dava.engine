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
    PlatformCore(EngineBackend* engineBackend);
    ~PlatformCore();

    NativeService* GetNativeService() const;

    void Init();
    void Run();
    void PrepareToQuit();
    void Quit();

private:
    WindowBackend* ActivityOnCreate();
    void ActivityOnResume();
    void ActivityOnPause();
    void ActivityOnDestroy();

    void GameThread();

private:
    EngineBackend* engineBackend = nullptr;
    MainDispatcher* mainDispatcher = nullptr;
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
