#if defined(__DAVAENGINE_COREV2__)

#pragma once

#if defined(__DAVAENGINE_WIN32__) && !defined(__DAVAENGINE_QT__)
#include "Engine/Private/Win32/WindowWin32.h"
#elif defined(__DAVAENGINE_QT__)
#include "Engine/Private/Qt/WindowQt.h"
#else
#error "PlatformWindow is not implemented yet"
#endif

#endif // __DAVAENGINE_COREV2__
