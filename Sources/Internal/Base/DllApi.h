#pragma once

#include "Base/Platform.h"

#ifdef __DAVAENGINE_WINDOWS__
#ifdef DAVA_ENGINE_EXPORTS
#define DAVA_ENGINE_API __declspec(dllexport)
#else
#define DAVA_ENGINE_API __declspec(dllimport)
#endif
#else
#define DAVA_ENGINE_API __attribute__((visibility("default")))
#endif

#ifdef __DAVAENGINE_WINDOWS__
#ifdef DAVA_MODULE_EXPORTS
#define DAVA_MODULE_API __declspec(dllexport)
#else
#define DAVA_MODULE_API __declspec(dllimport)
#endif
#else
#define DAVA_MODULE_API __attribute__((visibility("default")))
#endif
