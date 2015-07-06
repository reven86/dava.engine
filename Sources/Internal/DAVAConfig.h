/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


/*
	__DAVAENGINE_IPHONE__ this define must be set in preprocessor macros for all projects that compiled using DAVAEngine for iPhone
 */

#ifndef __DAVAENGINE_CONFIG_H__
#define __DAVAENGINE_CONFIG_H__

#ifndef __DAVAENGINE_AUTOTESTING__
#   define __DAVAENGINE_AUTOTESTING__
#endif
#ifndef AUTOTESTING_LUA
#   define AUTOTESTING_LUA
#endif

//#define ENABLE_BASE_OBJECT_CHECKS // separate thing to check if you release BaseObjects properly. Need to be disabled for release configurations 

//#define ENABLE_CONTROL_EDIT //allows to drug'n'drop controls for position editing

//#define SHOW_FRAME_TIME	// shows milliseconds per fame

//#define LOCALIZATION_DEBUG // enable graphic debugging info for displaying of text 

//#define __DAVAENGINE_RENDER_AUTOCONFIG__	// it will use DAVANENGINE_OPENGL for MacOS / iPhone, and 
//#define __DAVAENGINE_DIRECTX9__
#define __DAVAENGINE_OPENGL__

// This flag allow to enable profiling stats 
//#define __DAVAENGINE_ENABLE_DEBUG_STATS__

//suppress 'deprecated' warning
#define DAVAENGINE_HIDE_DEPRECATED

// Switch on/off message box in assertion situation. In case this flag is
// enabled the assertion message will be displayed even in release builds.
#if defined(__DAVAENGINE_DEBUG__)   //always enable full DVASSERT service for debug configurations
#   define ENABLE_ASSERT_LOGGING
#   define ENABLE_ASSERT_MESSAGE
#   define ENABLE_ASSERT_BREAK
#else //not defined __DAVAENGINE_DEBUG__    // can disable or select any dvassert service
#   define ENABLE_ASSERT_LOGGING
#   define ENABLE_ASSERT_MESSAGE
//    #define ENABLE_ASSERT_BREAK
#endif //

#define USE_FILEPATH_IN_MAP
#ifdef USE_FILEPATH_IN_MAP
#   define FILEPATH_MAP_KEY(key) key
#else //#ifdef USE_FILEPATH_IN_MAP
#   define FILEPATH_MAP_KEY(key) key.GetAbsolutePathname()
#endif //#ifdef USE_FILEPATH_IN_MAP

#define REBUILD_TANGENT_SPACE_ON_IMPORT

//Uncomment this define to using C++11 concurrency instead native
//#define USE_CPP11_CONCURRENCY

#endif // __DAVAENGINE_CONFIG_H__

