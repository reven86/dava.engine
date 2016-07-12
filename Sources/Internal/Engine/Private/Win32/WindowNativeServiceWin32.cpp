#include "Engine/Public/Win32/WindowNativeServiceWin32.h"

#if defined(__DAVAENGINE_COREV2__)

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_WIN32__)

#include "Engine/Private/Win32/WindowWin32.h"

namespace DAVA
{
WindowNativeService::WindowNativeService(Private::WindowWin32* w)
    : nativeWindow(w)
{
}

} // namespace DAVA

#endif // __DAVAENGINE_WIN32__
#endif // __DAVAENGINE_COREV2__
