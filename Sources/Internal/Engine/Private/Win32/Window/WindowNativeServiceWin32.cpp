#include "Engine/Win32/WindowNativeServiceWin32.h"

#if defined(__DAVAENGINE_COREV2__)

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_WIN32__)

#include "Engine/Private/Win32/Window/WindowBackendWin32.h"

namespace DAVA
{
WindowNativeService::WindowNativeService(Private::WindowBackend* windowBackend)
    : windowBackend(windowBackend)
{
}

HWND WindowNativeService::GetHWND() const
{
    return windowBackend->GetHWND();
}

} // namespace DAVA

#endif // __DAVAENGINE_WIN32__
#endif // __DAVAENGINE_COREV2__
