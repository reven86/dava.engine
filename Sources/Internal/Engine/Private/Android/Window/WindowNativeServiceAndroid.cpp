#include "Engine/Public/Android/WindowNativeServiceAndroid.h"

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_ANDROID__)

#include "Engine/Private/Android/Window/WindowBackendAndroid.h"

namespace DAVA
{
WindowNativeService::WindowNativeService(Private::WindowBackend* wbackend)
    : windowBackend(wbackend)
{
}

jobject WindowNativeService::CreateNativeControl(const char8* controlClassName, void* backendPointer)
{
    return windowBackend->CreateNativeControl(controlClassName, backendPointer);
}

} // namespace DAVA

#endif // __DAVAENGINE_ANDROID__
#endif // __DAVAENGINE_COREV2__
