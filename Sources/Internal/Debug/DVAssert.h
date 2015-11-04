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


#ifndef __DAVAENGINE_ASSERT_H__
#define __DAVAENGINE_ASSERT_H__

#include "Base/BaseTypes.h"
#include "FileSystem/Logger.h"
#include "Utils/StringFormat.h"

/**
	\page tutorial_debug Debugging
	Here you'll learn how to debug your project and find bugs faster.

	\section asserts Assertion macros 
	For debugging purposes and for easy search of bugs in runtime you can use assert macros. 
	There are 2 types of macros defined: DVASSERT and DVVERIFY.

	DVASSERT macro designed for situations where you want to check something but in release you want to remove this check at all. 

	For example, you have a function SetFrame and frame can't be negative, but you want to check it only in debug, and stop execution if such situation happened. 
	You can write
	\code
	void SetFrame(int32 frame)
	{
		DVASSERT(frame >= 0);		// this code will be removed in release configuration.

		// Function code
	}
	\endcode

	In case if you execute some function inside your assertion and want to leave the calls but remove checks you should 
	use DVVERIFY macro. 

	\code
	void SomeFunction(BaseObject * object)
	{
		int32 propertyInt;
		DVVERIFY(GetObjectProperty(object, "propertyInt", &propertyInt));		// this code will not be removed in release configuration.
	}
	\endcode
*/


// Runtime assert
#include "Debug/DVAssertMessage.h"


#if defined(ENABLE_ASSERT_BREAK)

#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_ANDROID__) // Mac & iPhone & Android

#include <signal.h>
#include <unistd.h>

#endif //PLATFORMS

inline void DavaDebugBreak()
{
#if defined(__DAVAENGINE_WINDOWS__)

    __debugbreak();

#elif defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_ANDROID__) // Mac & iPhone & Android

    raise(SIGTRAP);

#else //PLATFORMS
#   error "DavaDebugBreak: undefined platform"
#endif //PLATFORMS
}

#else

#define DavaDebugBreak() 

#endif  //__DAVAENGINE_DEBUG__


#if defined(ENABLE_ASSERT_LOGGING)
	// end=assert=msg - used as marker on teamcity to fail build
    #define LogErrorFunction(assertType, expr, msg, file, line) { DAVA::Logger::Error    ("========================================\n%s\n%s\n%s\nFile: %s\nLine: %d\n======================end=assert=msg====", assertType, expr, msg, file, line); }
    #define LogWarningFunction(assertType, expr, msg, file, line) { DAVA::Logger::Warning("========================================\n%s\n%s\n%s\nFile: %s\nLine: %d\n========================================", assertType, expr, msg, file, line); }
#else //ENABLE_ASSERT_LOGGING
    #define LogErrorFunction(assertType, expr, msg, file, line)
    #define LogWarningFunction(assertType, expr, msg, file, line)
#endif //ENABLE_ASSERT_LOGGING

#if defined(ENABLE_ASSERT_MESSAGE)
    #define MessageFunction(messagetype, assertType, expr, msg, file, line) \
        DAVA::DVAssertMessage::ShowMessage(messagetype, \
            "%s\n\n%s\n%s\n\nFile: %s\nLine: %d", assertType, expr, msg, file, \
            line)
#else //ENABLE_ASSERT_MESSAGE
    #define MessageFunction(messagetype, assertType, expr, msg, file, line) \
        false
#endif //ENABLE_ASSERT_MESSAGE




#if !defined(__DAVAENGINE_DEBUG__) && !defined(ENABLE_ASSERT_MESSAGE) && !defined(ENABLE_ASSERT_LOGGING) && !defined(ENABLE_ASSERT_BREAK)

	// no assert functions in release builds
	#define DVASSERT(expr) {}
	#define DVASSERT_MSG(expr, msg) {}
	#define DVWARNING(expr, msg) {}

	#define DVVERIFY(expr) do {(void)(expr);} while(false);

#else

// uncomment exit(-1) to shut up static analyzer (null pointers usage)
#define DV_EXIT_ON_ASSERT // exit(-1);

#define DVASSERT(expr)\
    if (!(expr))\
    {\
        LogErrorFunction("DV_ASSERT", #expr, "", __FILE__, __LINE__);\
        if (MessageFunction(DAVA::DVAssertMessage::ALWAYS_MODAL, "DV_ASSERT", \
                            #expr, "", __FILE__, __LINE__))\
        { \
            DavaDebugBreak();\
        } \
		DV_EXIT_ON_ASSERT \
	}\

#define DVASSERT_MSG(expr, msg)\
    if (!(expr))\
    {\
        LogErrorFunction("DV_ASSERT", #expr, msg, __FILE__, __LINE__);\
        if (MessageFunction(DAVA::DVAssertMessage::ALWAYS_MODAL, "DV_ASSERT", \
                            #expr, msg, __FILE__, __LINE__))\
        { \
            DavaDebugBreak();\
        } \
		DV_EXIT_ON_ASSERT \
    }\

#define DVWARNING(expr, msg)\
    if (!(expr))\
    {\
        LogWarningFunction("DV_WARNING", #expr, msg, __FILE__, __LINE__);\
		MessageFunction(DAVA::DVAssertMessage::TRY_NONMODAL, "DV_WARNING", \
		        #expr, msg, __FILE__, __LINE__);\
    }\

#define DVVERIFY(expr) DVASSERT(expr)

#endif// ndef __DAVAENGINE_DEBUG__ && ndef ENABLE_ASSERT_MESSAGE

#endif// __LOGENGINE_ASSERT_H__

