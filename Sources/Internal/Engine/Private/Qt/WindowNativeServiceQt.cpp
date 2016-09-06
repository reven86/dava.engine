#include "Engine/Public/Qt/WindowNativeServiceQt.h"

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)

#include "Engine/Private/Qt/WindowBackendQt.h"

namespace DAVA
{
WindowNativeService::WindowNativeService(Private::WindowBackend* wbackend)
    : windowBackend(wbackend)
{
}

void WindowNativeService::AcqureContext()
{
    windowBackend->AcqureContext();
}

void WindowNativeService::ReleaseContext()
{
    windowBackend->ReleaseContext();
}

} // namespace DAVA

#endif // __DAVAENGINE_QT__
#endif // __DAVAENGINE_COREV2__