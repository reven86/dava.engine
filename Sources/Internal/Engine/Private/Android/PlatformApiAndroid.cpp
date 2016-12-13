#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_ANDROID__)

#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Android/Window/WindowBackendAndroid.h"

namespace DAVA
{
namespace PlatformApi
{
namespace Android
{
jobject CreateNativeControl(Window* targetWindow, const char8* controlClassName, void* backendPointer)
{
    using namespace DAVA::Private;
    WindowBackend* wb = EngineBackend::GetWindowBackend(targetWindow);
    return wb->CreateNativeControl(controlClassName, backendPointer);
}

} // namespace Android
} // namespace PlatformApi
} // namespace DAVA

#endif // defined(__DAVAENGINE_ANDROID__)
#endif // defined(__DAVAENGINE_COREV2__)
