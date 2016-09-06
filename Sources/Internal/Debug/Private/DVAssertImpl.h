#pragma once

#ifndef __DAVA_DVASSERT_H__
#include "Debug/DVAssert.h"
#endif

static DAVA::Assert::FailBehaviour HandleAssert(const char* const expr,
                                                const char* const fileName,
                                                const int lineNumber,
                                                const DAVA::Vector<DAVA::Debug::StackFrame> backtrace,
                                                const char* const message)
{
    const DAVA::Vector<DAVA::Assert::Handler>& handlers = DAVA::Assert::GetHandlers();

    // If no handlers were registred, just halt
    if (handlers.empty())
    {
        return DAVA::Assert::FailBehaviour::Halt;
    }

    // Invoke all the handlers with according assert info
    // Return FailBehaviour::Halt if at least one of them requested that

    const DAVA::Assert::AssertInfo assertInfo(expr, fileName, lineNumber, message, backtrace);

    bool halt = false;
    for (const DAVA::Assert::Handler& handler : handlers)
    {
        const DAVA::Assert::FailBehaviour handlerFailBehaviour = handler(assertInfo);
        halt |= (handlerFailBehaviour == DAVA::Assert::FailBehaviour::Halt);
    }

    return halt ? DAVA::Assert::FailBehaviour::Halt : DAVA::Assert::FailBehaviour::Continue;
}