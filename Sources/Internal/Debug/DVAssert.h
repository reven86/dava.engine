#pragma once

// TODO: get rid of this dependency
#include "Debug/Backtrace.h"

/// \defgroup Asserts
/// Asserts are a set of macroses for testing conditions which are always expected to be true.
///
/// There are two of them defined:
/// DVASSERT - turned on in Debug mode, turned off for Release mode by default (will be fully stripped including expression it checks).
///            It can also be left turned on in Release mode using __DAVAENGINE_ENABLE_ASSERTS__
/// DVASSERT_ALWAYS - turned on in both Debug and Release mode
///
/// They both have the same interface and can be called with or without additional message
///
/// Usage examples:
/// - DVASSERT(isConnected)
/// - DVASSERT_ALWAYS(isConnected, "User is not connected!")
///
/// If an assert fails, default behaviour is to halt a program (and show the line during debugging),
/// but adding your own handlers is supported via DAVA::Assert::AddHandler (and removing with DAVA::Assert::RemoveHandler).
/// Each handler accepts DAVA::Assert::AssertInfo object with all the info and should return one of three values:
/// - DAVA::Assert::FailBehaviour::Default if handler either doesn't know what to do or is not responsible for that
/// - DAVA::Assert::FailBehaviour::Continue to indicate that program should not be stopped because of an assert
/// - DAVA::Assert::FailBehaviour::Halt otherwise
///
/// All the handlers will be called every time, even if one of them has already reported FailBehaviour::Halt.
///
/// A good example would be adding two handlers: the first one logs the assert somewhere and returns FailBehaviour::Default,
/// (since it doesn't know what to do with the assert and just logs it), and second one shows a dialog box asking user
/// if a program should be stopped and returning FailBehaviour::Halt in case it should.
///
namespace DAVA
{
/// \ingroup Asserts
namespace Assert
{
/// Helper class that groups information about an assert
class AssertInfo final
{
public:
    AssertInfo(const char* const expression,
               const char* const fileName,
               const int lineNumber,
               const char* const message,
               Vector<Debug::StackFrame> backtrace)
        : expression(expression)
        , fileName(fileName)
        , lineNumber(lineNumber)
        , message(message)
        , backtrace(std::move(backtrace))
    {
    }

    const char* const expression;
    const char* const fileName;
    const int lineNumber;
    const char* const message;
    const Vector<Debug::StackFrame> backtrace;
};

/// Indicates how a program should act when an assert fails
enum class FailBehaviour
{
    /// Default behaviour
    Default,

    /// Ignore an assert and continue invocation
    Continue,

    /// Stop invocation
    Halt
};

/// Function that handles failed asserts and returns FailBehaviour
/// \param[in] assertInfo Information about a failed assert
/// \returns FailBehaviour value, indicating if a program should be halted or not
using Handler = FailBehaviour (*)(const AssertInfo& assertInfo);

/// Registers a handler to use
/// \param[in] handler Handler to add
void AddHandler(const Handler handler);

/// Unregisters a handler if it was added before
/// \param[in] handler Handler to remove
void RemoveHandler(const Handler handler);
}
}

// Macro for generating debug break
// It's not a function on Windows in order to prevent stacktrace altering
// TODO: release behaviour?
#if defined(__DAVAENGINE_WINDOWS__)
#define DVASSERT_HALT() __debugbreak()
#elif defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_ANDROID__)
void RaiseSigTrap();
#define DVASSERT_HALT() RaiseSigTrap();
#else
#error "DVASSERT_HALT is not defined for current platform"
#endif

/// Used internally by DVASSERT_INTERNAL macro
DAVA::Assert::FailBehaviour HandleAssert(const char* const expr,
                                         const char* const fileName,
                                         const int lineNumber,
                                         const DAVA::Vector<DAVA::Debug::StackFrame>& backtrace,
                                         const char* const message = "");

// Common macro to use by DVASSERT & DVASSERT_ALWAYS to avoid code duplication
#define DVASSERT_INTERNAL(expr, ...) \
    do \
    { \
        if (!(expr)) \
        { \
            if (HandleAssert( \
                #expr, \
                __FILE__, \
                __LINE__, \
                DAVA::Debug::GetBacktrace(), \
                ##__VA_ARGS__) != DAVA::Assert::FailBehaviour::Continue) \
            { \
                DVASSERT_HALT(); \
            } \
        } \
    } \
    while (false)

#if defined(__DAVAENGINE_DEBUG__) || defined(__DAVAENGINE_ENABLE_ASSERTS__)

#define DVASSERT(expr, ...) DVASSERT_INTERNAL(expr, ##__VA_ARGS__)

#else

// Tricking compiler to think this expr is actually being used without calculating it
#define DVASSERT_UNUSED(expr) (void)(true ? (void)0 : ((void)(expr)))

#define DVASSERT(expr, ...) DVASSERT_UNUSED(expr)

#endif

#define DVASSERT_ALWAYS(expr, ...) DVASSERT_INTERNAL(expr, ##__VA_ARGS__)