/*
	__DAVAENGINE_IPHONE__ this define must be set in preprocessor macros for all projects that compiled using DAVAEngine for iPhone
 */

#ifndef __DAVAENGINE_CONFIG_H__
#define __DAVAENGINE_CONFIG_H__

#ifndef DISABLE_AUTOTESTS
#ifndef __DAVAENGINE_AUTOTESTING__
#define __DAVAENGINE_AUTOTESTING__
#endif
#ifndef AUTOTESTING_LUA
#define AUTOTESTING_LUA
#endif
#endif

// Enable memory profiling - should be defined for non-cmake projects
//#define DAVA_MEMORY_PROFILING_ENABLE

//#define ENABLE_BASE_OBJECT_CHECKS // separate thing to check if you release BaseObjects properly. Need to be disabled for release configurations

//#define ENABLE_CONTROL_EDIT //allows to drug'n'drop controls for position editing

//#define SHOW_FRAME_TIME	// shows milliseconds per fame

//#define LOCALIZATION_DEBUG // enable graphic debugging info for displaying of text

//#define __DAVAENGINE_RENDER_AUTOCONFIG__	// it will use DAVANENGINE_OPENGL for MacOS / iPhone, and
//#define __DAVAENGINE_DIRECTX9__
#define __DAVAENGINE_OPENGL__

// This flag allow to enable profiling stats
//#define __DAVAENGINE_ENABLE_DEBUG_STATS__
#if defined(__DAVAENGINE_ENABLE_DEBUG_STATS__)
//    #define __DAVAENGINE_ENABLE_FRAMEWORK_STATS__
//    #define __DAVAENGINE_ENABLE_TOOLS_STATS__
#endif //__DAVAENGINE_ENABLE_DEBUG_STATS__

//suppress 'deprecated' warning
#define DAVAENGINE_HIDE_DEPRECATED

// Switch on/off message box in assertion situation. In case this flag is
// enabled the assertion message will be displayed even in release builds.
#if defined(__DAVAENGINE_DEBUG__) //always enable full DVASSERT service for debug configurations
#define ENABLE_ASSERT_LOGGING
#define ENABLE_ASSERT_MESSAGE
#define ENABLE_ASSERT_BREAK
//#define DAVA_ENGINE_DEBUG_FPU_EXCEPTIONS // throw std::runtime_error on div_by_zero, overflow, underflow
#else //not defined __DAVAENGINE_DEBUG__    // can disable or select any dvassert service
#define ENABLE_ASSERT_LOGGING
#define ENABLE_ASSERT_MESSAGE
//    #define ENABLE_ASSERT_BREAK
#endif //

#define USE_FILEPATH_IN_MAP
#ifdef USE_FILEPATH_IN_MAP
#define FILEPATH_MAP_KEY(key) key
#else //#ifdef USE_FILEPATH_IN_MAP
#define FILEPATH_MAP_KEY(key) key.GetAbsolutePathname()
#endif //#ifdef USE_FILEPATH_IN_MAP

//Uncomment this define to using C++11 concurrency instead native
//#define USE_CPP11_CONCURRENCY

#endif // __DAVAENGINE_CONFIG_H__
