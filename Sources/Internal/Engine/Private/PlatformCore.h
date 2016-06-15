#if defined(__DAVAENGINE_COREV2__)

#pragma once

#if defined(__DAVAENGINE_QT__)
#include "Engine/Private/Qt/CoreQt.h"
#elif defined(__DAVAENGINE_WIN32__)
#include "Engine/Private/Win32/CoreWin32.h"
#elif defined(__DAVAENGINE_MACOS__)
#include "Engine/Private/OsX/CoreOsX.h"
#else
#if defined(__DAVAENGINE_COREV2__)
// Do not emit error when building with old core implementation
#error "PlatformCore is not implemented"
#endif
#endif

#endif // __DAVAENGINE_COREV2__
