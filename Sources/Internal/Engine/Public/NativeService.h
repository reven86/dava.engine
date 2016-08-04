#pragma once

// Select right header for NativeService class depending on platfrom
#if defined(__DAVAENGINE_QT__)
#include "Engine/Public/Qt/NativeServiceQt.h"
#elif defined(__DAVAENGINE_WIN32__)
#include "Engine/Public/Win32/NativeServiceWin32.h"
#elif defined(__DAVAENGINE_WIN_UAP__)
#include "Engine/Public/UWP/NativeServiceUWP.h"
#elif defined(__DAVAENGINE_MACOS__)
#include "Engine/Public/OsX/NativeServiceOsX.h"
#else
#if defined(__DAVAENGINE_COREV2__)
// Do not emit error when building with old core implementation
#error "NativeService is not implemented"
#endif
#endif
