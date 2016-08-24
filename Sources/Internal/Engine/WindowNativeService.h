#pragma once

// Select right header for WindowNativeService class depending on platfrom
#if defined(__DAVAENGINE_QT__)
#include "Engine/Qt/WindowNativeServiceQt.h"
#elif defined(__DAVAENGINE_WIN32__)
#include "Engine/Win32/WindowNativeServiceWin32.h"
#elif defined(__DAVAENGINE_WIN_UAP__)
#include "Engine/UWP/WindowNativeServiceUWP.h"
#elif defined(__DAVAENGINE_MACOS__)
#include "Engine/OsX/WindowNativeServiceOsX.h"
#elif defined(__DAVAENGINE_IPHONE__)
#include "Engine/iOS/WindowNativeServiceiOS.h"
#elif defined(__DAVAENGINE_ANDROID__)
#include "Engine/Android/WindowNativeServiceAndroid.h"
#else
#if defined(__DAVAENGINE_COREV2__)
// Do not emit error when building with old core implementation
#error "WindowNativeService is not implemented"
#endif
#endif
