#pragma once

// Select right header for NativeService class depending on platform
#if defined(__DAVAENGINE_QT__)
#include "Engine/Qt/NativeServiceQt.h"
#elif defined(__DAVAENGINE_WIN32__)
#include "Engine/Win32/NativeServiceWin32.h"
#elif defined(__DAVAENGINE_WIN_UAP__)
#include "Engine/UWP/NativeServiceUWP.h"
#elif defined(__DAVAENGINE_MACOS__)
#include "Engine/OsX/NativeServiceOsX.h"
#elif defined(__DAVAENGINE_IPHONE__)
#include "Engine/iOS/NativeServiceiOS.h"
#elif defined(__DAVAENGINE_ANDROID__)
#include "Engine/Android/NativeServiceAndroid.h"
#else
#if defined(__DAVAENGINE_COREV2__)
// Do not emit error when building with old core implementation
#error "NativeService is not implemented"
#endif
#endif
