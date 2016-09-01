#pragma once

#include <tuple>
#include "Debug/Backtrace.h"

/** \defgroup Asserts
* Asserts are a set of macroses for testing conditions which are always expected to be true.
*
* There are two of them:
* DVASSERT - turned on in Debug mode, turned off for Release mode
* DVASSERT_CRITICAL - turned on in both Debug and Release mode
*
* They both have the same interface and can be called with or without additional message
* (message formatting is supported via additional arguments).
*
* Usage examples:
* - DVASSERT(isConnected)
* - DVASSERT(isConnected, "User is not connected!")
* - DVASSERT_CRITICAL(itemId == 100, "Item id was expected to be equal to 100, but it's equal to: %d", itemId)
*
* If an assert fails, default behaviour is to halt a program (and show the line during debugging),
* but adding your own handlers is supported via DAVA::Assert::AddHandler (and removing with DAVA::Assert::RemoveHandler).
* Each handler accepts DAVA::Assert::AssertInfo object with all the info and should return one of two values:
* - DAVA::Assert::FailBehaviour::Continue to indicate that program should not be stopped beucause of an assert
* - DAVA::Assert::FailBehaviour::Halt otherwise
*
* All the handlers will be called every time, even if one of them has already reported FailBehaviour::Halt.
*
* Note that if at least one handler is added, you opt for controlling when a program should be halted all by yourself,
* since default behaviour (always stopping a program) will no longer be used.
*
* A good example would be adding two handlers: the first one logs the assert somewhere and returns FailBehaviour::Continue,
* and second one shows a dialog box asking user if a program should be stopped and returning FailBehaviour::Halt in case it should.
*/

namespace DAVA
{
/** \ingroup Asserts */
namespace Assert
{    
    /** Helper class that groups information about an assert */
	class AssertInfo final
	{
	public:
		AssertInfo(
			const char* const expression,
			const char* const fileName,
			const int lineNumber,
			const char* const message,
			const DAVA::Vector<DAVA::Debug::StackFrame> backtrace)
			: expression(expression)
			, fileName(fileName)
			, lineNumber(lineNumber)
			, message(message)
			, backtrace(backtrace)
		{

		}

		const char* const expression;
		const char* const fileName;
		const int lineNumber;
        const char* const message;
		const DAVA::Vector<DAVA::Debug::StackFrame> backtrace;
	};

	/** Indicates how a program should act when an assert fails */
	enum class FailBehaviour
	{
		/** Ignore an assert and continue invocation */
		Continue,

		/** Stop invocation */
		Halt
	};

	/** Typedef for function that handles failed asserts and returns FailBehaviour
    * \param[in] assertInfo Information about a failed assert
    * \returns FailBehaviour value, indicating if a program should be halted or not
    */
	typedef FailBehaviour (*Handler)(const AssertInfo& assertInfo);

	/** Registers a handler to use
    * \param[in] handler Handler to add
    */
	void AddHandler(const Handler handler);

	/** Unregisters a handler if it was added before
    * \param[in] handler Handler to remove
    */
	void RemoveHandler(const Handler handler);

	/** Handles a failed assert (used internally by assert macroses) */
	FailBehaviour Handle(
        const char* const expr,
        const char* const fileName,
        const int lineNumber,
        const DAVA::Vector<DAVA::Debug::StackFrame> backtrace,
        const int varArgsCount,
        ...);
}
}

// Macro for generating debug break
// It's not a function in order to prevent stacktrace altering
#if defined (__DAVAENGINE_WINDOWS__)
#define DVASSERT_HALT __debugbreak()
#elif defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_ANDROID__)
#define DVASSERT_HALT raise(SIGTRAP)
#else
#error "DVASSERT_HALT is not defined for current platform"
#endif

// Helper macro to get count of __VA_ARGS__
// Required to distinguish between asserts with and without messages
#define DVASSERT_ARGS_COUNT(...) std::tuple_size<decltype(std::make_tuple(__VA_ARGS__))>::value

// Uncomment exit(-1) to shut up static analyzer (null pointers usage)
#define DV_EXIT_ON_ASSERT // exit(-1);

#define DVASSERT_INTERNAL(expr, ...) \
    do \
    { \
        if (!(expr)) \
        { \
            if (DAVA::Assert::Handle( \
                    #expr, \
                    __FILE__, \
                    __LINE__, \
                    DAVA::Debug::GetBacktrace(), \
                    DVASSERT_ARGS_COUNT(__VA_ARGS__), \
                    __VA_ARGS__) == DAVA::Assert::FailBehaviour::Halt) \
            { \
                DVASSERT_HALT; \
            } \
            DV_EXIT_ON_ASSERT \
        } \
    } \
    while (false)

#if defined(__DAVAENGINE_DEBUG__)

#define DVASSERT(expr, ...) DVASSERT_INTERNAL(expr, __VA_ARGS__)

#else

// Tricking compiler to think this expr is actually being used without calculating it
#define DVASSERT_UNUSED(expr) (void)(true ? (void)0 : ((void)(expr)))

#define DVASSERT(expr, ...) DVASSERT_UNUSED(expr)

#endif

#define DVASSERT_CRITICAL(expr, ...) DVASSERT_INTERNAL(expr, __VA_ARGS__)