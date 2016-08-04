#pragma once

// Select right header for WindowNativeService class depending on platfrom
#if defined(__DAVAENGINE_QT__)
#include "Engine/Public/Qt/WindowNativeServiceQt.h"
#elif defined(__DAVAENGINE_WIN32__)
#include "Engine/Public/Win32/WindowNativeServiceWin32.h"
#elif defined(__DAVAENGINE_WIN_UAP__)
#include "Engine/Public/UWP/WindowNativeServiceUWP.h"
#elif defined(__DAVAENGINE_MACOS__)
#include "Engine/Public/OsX/WindowNativeServiceOsX.h"
#elif defined(__DAVAENGINE_IPHONE__)
#include "Engine/Public/iOS/WindowNativeServiceiOS.h"
#else
#if defined(__DAVAENGINE_COREV2__)
// Do not emit error when building with old core implementation
#error "WindowNativeService is not implemented"
#endif
#endif
