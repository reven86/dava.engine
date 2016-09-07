#include "Debug/DVAssertHandlers.h"
#include "Debug/DVAssertMessage.h"
#include "Logger/Logger.h"

namespace DAVA
{
namespace Assert
{
FailBehaviour LoggerHandler(const AssertInfo& assertInfo)
{
    Logger::Error(
    "========================================\n"
    "%s\n"
    "Expression: %s\n"
    "Message: %s\n"
    "At %s:%d\n"
    "========================================",
    AssertMessageTag.c_str(), assertInfo.expression, assertInfo.message, assertInfo.fileName, assertInfo.lineNumber);

    Debug::BacktraceToLog(assertInfo.backtrace, Logger::LEVEL_ERROR);

    return FailBehaviour::Continue;
}

FailBehaviour DialogBoxHandler(const AssertInfo& assertInfo)
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
    "%s\n"
    "Expression: %s\n"
    "Message: %s\n"
    "At %s:%d\n"
    "Callstack:\n"
    "%s",
    AssertMessageTag.c_str(), assertInfo.expression, assertInfo.message, assertInfo.fileName, assertInfo.lineNumber,
    Debug::BacktraceToString(assertInfo.backtrace, backtraceDepth).c_str());

    return halt ? FailBehaviour::Halt : FailBehaviour::Continue;
}
}
}
