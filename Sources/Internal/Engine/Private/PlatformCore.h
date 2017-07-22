#pragma once

#if defined(__DAVAENGINE_COREV2__)

#if defined(__DAVAENGINE_QT__)
#include "Engine/Private/Qt/PlatformCoreQt.h"
#elif defined(__DAVAENGINE_WIN32__)
#include "Engine/Private/Win32/PlatformCoreWin32.h"
#elif defined(__DAVAENGINE_WIN_UAP__)
#include "Engine/Private/UWP/PlatformCoreUWP.h"
#elif defined(__DAVAENGINE_MACOS__)
#include "Engine/Private/OsX/PlatformCoreOsX.h"
#elif defined(__DAVAENGINE_IPHONE__)
#include "Engine/Private/iOS/PlatformCoreiOS.h"
#elif defined(__DAVAENGINE_ANDROID__)
#include "Engine/Private/Android/PlatformCoreAndroid.h"
#elif defined(__DAVAENGINE_LINUX__)
#include "Engine/Private/Linux/PlatformCoreLinux.h"
#else
#if defined(__DAVAENGINE_COREV2__)
// Do not emit error when building with old core implementation
#error "PlatformCore is not implemented"
#endif
#endif

#endif // __DAVAENGINE_COREV2__
