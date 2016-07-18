#if defined(__DAVAENGINE_COREV2__)

#pragma once

#if defined(__DAVAENGINE_QT__)
#include "Engine/Private/Qt/WindowBackendQt.h"
#elif defined(__DAVAENGINE_WIN32__)
#include "Engine/Private/Win32/Window/WindowBackendWin32.h"
#elif defined(__DAVAENGINE_WIN_UAP__)
#include "Engine/Private/UWP/Window/WindowBackendUWP.h"
#elif defined(__DAVAENGINE_MACOS__)
#include "Engine/Private/OsX/Window/WindowBackendOsX.h"
#elif defined(__DAVAENGINE_IPHONE__)
#include "Engine/Private/iOS/Window/WindowBackendiOS.h"
#else
#if defined(__DAVAENGINE_COREV2__)
// Do not emit error when building with old core implementation
#error "PlatformWindow is not implemented"
#endif
#endif

#endif // __DAVAENGINE_COREV2__
