#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/Android/PlatformCoreAndroid.h"

#if defined(__DAVAENGINE_ANDROID__)

#include "Engine/Public/Android/NativeServiceAndroid.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Android/Window/WindowBackendAndroid.h"

namespace DAVA
{
namespace Private
{
PlatformCore::PlatformCore(EngineBackend* ebackend)
    : engineBackend(ebackend)
    , nativeService(new NativeService(this))
{
}

PlatformCore::~PlatformCore() = default;

void PlatformCore::Init()
{
}

void PlatformCore::Run()
{
}

void PlatformCore::Quit()
{
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_ANDROID__
#endif // __DAVAENGINE_COREV2__
