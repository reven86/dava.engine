#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_WIN32__)

#include "Engine/Private/EngineFwd.h"

namespace DAVA
{
class WindowNativeService final
{
public:
private:
    WindowNativeService(Private::WindowWin32* w);

private:
    Private::WindowWin32* nativeWindow = nullptr;

    // friends
    friend class Private::WindowWin32;
};

} // namespace DAVA

#endif // __DAVAENGINE_WIN32__
#endif // __DAVAENGINE_COREV2__
