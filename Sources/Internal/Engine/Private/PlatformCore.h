#if defined(__DAVAENGINE_COREV2__)

#pragma once

#if defined(__DAVAENGINE_WIN32__) && !defined(__DAVAENGINE_QT__)
#include "Engine/Private/Win32/CoreWin32.h"
#elif defined(__DAVAENGINE_QT__)
#include "Engine/Private/Qt/CoreQt.h"
#else
#error "PlatformCore is not implemented yet"
#endif

#endif // __DAVAENGINE_COREV2__
