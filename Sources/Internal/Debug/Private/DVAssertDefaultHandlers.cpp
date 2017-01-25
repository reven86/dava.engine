#include "Debug/DVAssertDefaultHandlers.h"
#include "Debug/DVAssertMessage.h"
#include "Debug/Backtrace.h"
#include "Logger/Logger.h"

namespace DAVA
{
namespace Assert
{
const String AssertMessageTag = "end=assert=msg";

FailBehaviour DefaultLoggerHandler(const AssertInfo& assertInfo)
{
    Logger::Error(
    "========================================\n"
    "DVASSERT failed\n"
    "Expression: %s\n"
    "Message: %s\n"
    "At %s:%d\n"
    "======================%s====",
    assertInfo.expression, assertInfo.message, assertInfo.fileName, assertInfo.lineNumber, AssertMessageTag.c_str());

    DAVA::Logger::Error(
    "==== callstack ====\n"
    "%s\n"
    "==== callstack end ====",
    Debug::GetBacktraceString(assertInfo.backtrace).c_str());

    // Even though it's more appropriate to return FailBehaviour::Default,
    // return FailBehaviour::Continue to match behaviour of an old assert system
    return FailBehaviour::Continue;
}

FailBehaviour DefaultDialogBoxHandler(const AssertInfo& assertInfo)
{
// Android and iOS both allow content scrolling in assert dialog, so show full backtrace
// On desktops dialogs are not scrollable so limit frames
#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__)
    const int backtraceDepth = 8;
#else
    const int backtraceDepth = 0;
#endif

    const bool halt = DVAssertMessage::ShowMessage(
    DVAssertMessage::ALWAYS_MODAL,
    "DVASSERT failed\n"
    "Expression: %s\n"
    "Message: %s\n"
    "At %s:%d\n"
    "Callstack:\n"
    "%s",
    assertInfo.expression, assertInfo.message, assertInfo.fileName, assertInfo.lineNumber,
    Debug::GetBacktraceString(assertInfo.backtrace, backtraceDepth).c_str());

    return halt ? FailBehaviour::Halt : FailBehaviour::Continue;
}

void SetupDefaultHandlers()
{
    RemoveAllHandlers();

#ifdef ENABLE_ASSERT_LOGGING
    AddHandler(DefaultLoggerHandler);
#endif

#ifdef ENABLE_ASSERT_MESSAGE
    AddHandler(DefaultDialogBoxHandler);
#endif
}
}
}
