#ifndef __DAVAENGINE_ASSERT_H__
#define __DAVAENGINE_ASSERT_H__

#include "Base/BaseTypes.h"
#include "Logger/Logger.h"
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
#include "Debug/Backtrace.h"
#include "Logger/Logger.h"

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
#error "DavaDebugBreak: undefined platform"
#endif //PLATFORMS
}

#else

#define DavaDebugBreak()

#endif //__DAVAENGINE_DEBUG__

#if defined(ENABLE_ASSERT_LOGGING)
// end=assert=msg - used as marker on teamcity to fail build
#define LogErrorFunction(assertType, expr, msg, file, line, backtrace)                         \
    {                                                                                          \
        DAVA::Logger::Error( \
        "========================================\n" \
        "%s\n" \
        "%s\n" \
        "%s\n" \
        "at %s:%d\n" \
        "======================end=assert=msg====", \
        assertType, expr, msg, file, line); \
        DAVA::Logger::Error( \
        "==== callstack ====\n" \
        "%s\n" \
        "==== callstack end ====", \
        DAVA::Debug::BacktraceToString(backtrace).c_str()); \
    }
#define LogWarningFunction(assertType, expr, msg, file, line, backtrace)                       \
    {                                                                                          \
        DAVA::Logger::Warning( \
        "========================================\n" \
        "%s\n" \
        "%s\n" \
        "%s\n" \
        "at %s:%d\n" \
        "======================end=assert=msg====", \
        assertType, expr, msg, file, line); \
        DAVA::Logger::Warning( \
        "==== callstack ====\n" \
        "%s\n" \
        "==== callstack end ====", \
        DAVA::Debug::BacktraceToString(backtrace).c_str()); \
    }
#else //ENABLE_ASSERT_LOGGING
#define LogErrorFunction(assertType, expr, msg, file, line)
#define LogWarningFunction(assertType, expr, msg, file, line)
#endif //ENABLE_ASSERT_LOGGING

#if defined(ENABLE_ASSERT_MESSAGE)

// DAVA_BACKTRACE_DEPTH_UI tells how many stack frames show in assert dialog
// Android and ios both allow content scrolling in assert dialog, so show full backtrace
// On desktops dialogs are not scrollable so limit frames to 8
#ifndef DVASSERT_UI_BACKTRACE_DEPTH
#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__)
#define DVASSERT_UI_BACKTRACE_DEPTH 8
#else
#define DVASSERT_UI_BACKTRACE_DEPTH -1
#endif
#endif

#define MessageFunction(messagetype, assertType, expr, msg, file, line, backtrace) \
    DAVA::DVAssertMessage::ShowMessage(messagetype, \
                                       "%s\n" \
                                       "%s\n" \
                                       "%s\n" \
                                       "at %s:%d\n" \
                                       "Callstack:\n" \
                                       "%s", \
                                       assertType, expr, msg, file, line, \
                                       DAVA::Debug::BacktraceToString(backtrace, DVASSERT_UI_BACKTRACE_DEPTH).c_str())
#else //ENABLE_ASSERT_MESSAGE
#define MessageFunction(messagetype, assertType, expr, msg, file, line, backtrace) \
    false
#endif //ENABLE_ASSERT_MESSAGE

#if !defined(__DAVAENGINE_DEBUG__) && !defined(ENABLE_ASSERT_MESSAGE) && !defined(ENABLE_ASSERT_LOGGING) && !defined(ENABLE_ASSERT_BREAK)

// no assert functions in release builds
#define DVASSERT(expr) \
    {                  \
    }
#define DVASSERT_MSG(expr, msg) \
    {                           \
    }
#define DVWARNING(expr, msg) \
    {                        \
    }

#define DVVERIFY(expr) \
    do                 \
    {                  \
        (void)(expr);  \
    } while (false);

#else

// uncomment exit(-1) to shut up static analyzer (null pointers usage)
#define DV_EXIT_ON_ASSERT // exit(-1);

#define DVASSERT(expr)                                                                 \
    if (!(expr))                                                                       \
    {                                                                                  \
        DAVA::Vector<DAVA::Debug::StackFrame> backtrace = DAVA::Debug::GetBacktrace(); \
        LogErrorFunction("DV_ASSERT", #expr, "", __FILE__, __LINE__, backtrace);       \
        if (MessageFunction(DAVA::DVAssertMessage::ALWAYS_MODAL, "DV_ASSERT", \
                            #expr, "", __FILE__, __LINE__, backtrace))                 \
        {                                                                              \
            DavaDebugBreak();                                                          \
        }                                                                              \
        DV_EXIT_ON_ASSERT                                                              \
    }

#define DVASSERT_MSG(expr, msg)                                                        \
    if (!(expr))                                                                       \
    {                                                                                  \
        DAVA::Vector<DAVA::Debug::StackFrame> backtrace = DAVA::Debug::GetBacktrace(); \
        LogErrorFunction("DV_ASSERT", #expr, msg, __FILE__, __LINE__, backtrace);      \
        if (MessageFunction(DAVA::DVAssertMessage::ALWAYS_MODAL, "DV_ASSERT", \
                            #expr, msg, __FILE__, __LINE__, backtrace))                \
        {                                                                              \
            DavaDebugBreak();                                                          \
        }                                                                              \
        DV_EXIT_ON_ASSERT                                                              \
    }

#define DVWARNING(expr, msg)                                                           \
    if (!(expr))                                                                       \
    {                                                                                  \
        DAVA::Vector<DAVA::Debug::StackFrame> backtrace = DAVA::Debug::GetBacktrace(); \
        LogWarningFunction("DV_WARNING", #expr, msg, __FILE__, __LINE__, backtrace);   \
        MessageFunction(DAVA::DVAssertMessage::TRY_NONMODAL, "DV_WARNING", \
                        #expr, msg, __FILE__, __LINE__, backtrace);                    \
    }

#define DVVERIFY(expr) DVASSERT(expr)

#endif // ndef __DAVAENGINE_DEBUG__ && ndef ENABLE_ASSERT_MESSAGE

#endif // __LOGENGINE_ASSERT_H__
