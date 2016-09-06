#include "Debug/DVAssertHandlers.h"
#include "Debug/DVAssertMessage.h"
#include "Logger/Logger.h"

using namespace DAVA::Assert;

FailBehaviour DAVA::Assert::LoggerHandler(const AssertInfo& assertInfo)
{
    DAVA::Logger::Error(
        "========================================\n"
        "Assert failed\n"
        "Expression: %s\n" 
        "Message: %s\n" 
        "At %s:%d\n"
        "========================================", 
        assertInfo.expression, assertInfo.message, assertInfo.fileName, assertInfo.lineNumber);

    DAVA::Debug::BacktraceToLog(assertInfo.backtrace, DAVA::Logger::LEVEL_ERROR);

    return FailBehaviour::Continue;
}

FailBehaviour DAVA::Assert::DialogBoxHandler(const AssertInfo& assertInfo)
{
    // Android and iOS both allow content scrolling in assert dialog, so show full backtrace
    // On desktops dialogs are not scrollable so limit frames
#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__)
    const int backtraceDepth = 8;
#else
    const int backtraceDepth = -1;
#endif

    const bool halt = DAVA::DVAssertMessage::ShowMessage(
        DAVA::DVAssertMessage::ALWAYS_MODAL,
        "Assert failed\n"
        "Expression: %s\n"
        "Message: %s\n"
        "At %s:%d\n"
        "Callstack:\n"
        "%s",
        assertInfo.expression, assertInfo.message, assertInfo.fileName, assertInfo.lineNumber,
        DAVA::Debug::BacktraceToString(assertInfo.backtrace, backtraceDepth).c_str());

    return halt ? FailBehaviour::Halt : FailBehaviour::Continue;
}