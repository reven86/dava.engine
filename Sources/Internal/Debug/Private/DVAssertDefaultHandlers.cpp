#include "Debug/DVAssertDefaultHandlers.h"
#include "Debug/DVAssertMessage.h"
#include "Debug/Backtrace.h"
#include "Logger/Logger.h"

namespace DAVA
{
namespace Assert
{
FailBehaviour DefaultLoggerHandler(const AssertInfo& assertInfo)
{
    Logger::Error(
    "========================================\n"
    "Assert failed\n"
    "Expression: %s\n"
    "Message: %s\n"
    "At %s:%d\n"
    "======================%s====",
    assertInfo.expression, assertInfo.message, assertInfo.fileName, assertInfo.lineNumber, AssertMessageTag.c_str());

    Logger::Error(Debug::GetBacktraceString(assertInfo.backtrace).c_str());

    return FailBehaviour::Default;
}

FailBehaviour DefaultDialogBoxHandler(const AssertInfo& assertInfo)
{
// Android and iOS both allow content scrolling in assert dialog, so show full backtrace
// On desktops dialogs are not scrollable so limit frames
#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__)
    const int backtraceDepth = 8;
#else
    const int backtraceDepth = -1;
#endif

    const bool halt = DVAssertMessage::ShowMessage(
    DVAssertMessage::ALWAYS_MODAL,
    "Assert failed\n"
    "Expression: %s\n"
    "Message: %s\n"
    "At %s:%d\n"
    "Callstack:\n"
    "%s",
    assertInfo.expression, assertInfo.message, assertInfo.fileName, assertInfo.lineNumber,
    Debug::GetBacktraceString(assertInfo.backtrace).c_str());

    return halt ? FailBehaviour::Halt : FailBehaviour::Continue;
}
}
}
