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

    void Init();
    void Run();
    void PrepareToQuit();
    void Quit();

    void OnGamepadAdded(int32 deviceId, const String& name, bool hasTriggerButtons);
    void OnGamepadRemoved(int32 deviceId);

private:
    WindowBackend* ActivityOnCreate();
    void ActivityOnResume();
    void ActivityOnPause();
    void ActivityOnDestroy();

    void GameThread();

private:
    EngineBackend* engineBackend = nullptr;
    MainDispatcher* mainDispatcher = nullptr;

    bool quitGameThread = false;

    // Friends
    friend struct AndroidBridge;
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_ANDROID__
#endif // __DAVAENGINE_COREV2__
