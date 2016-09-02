#include "DVAssert.h"

#include <cstdarg>
#include <cstdio>

using namespace DAVA::Assert;

static const size_t formattedMessageBufferSize = 256;

static DAVA::Vector<Handler> registredHandlers;

void DAVA::Assert::AddHandler(const Handler handler)
{
    DAVA::Vector<Handler>::iterator position = std::find(registredHandlers.begin(), registredHandlers.end(), handler);
    if (position != registredHandlers.end())
    {
        return;
    }

    registredHandlers.push_back(handler);
}

void DAVA::Assert::RemoveHandler(const Handler handler)
{
    DAVA::Vector<Handler>::iterator position = std::find(registredHandlers.begin(), registredHandlers.end(), handler);
    if (position != registredHandlers.end())
    {
        registredHandlers.erase(position);
    }
}

FailBehaviour DAVA::Assert::Handle(
    const char* const expr,
    const char* const fileName,
    const int lineNumber,
    const DAVA::Vector<DAVA::Debug::StackFrame> backtrace,
    const int varArgsCount,
    ...)
{
    // If no handlers were registred, just halt
    if (registredHandlers.empty())
    {
        return FailBehaviour::Halt;
    }

    // Format message (if specified)
    char formattedMessage[formattedMessageBufferSize] = "";
    if (varArgsCount > 0)
    {
        va_list argsList;
        va_start(argsList, varArgsCount);

        const char* const messageFormat = va_arg(argsList, const char*);
        vsnprintf(formattedMessage, formattedMessageBufferSize, messageFormat, argsList);

        va_end(argsList);
    }

    // Invoke all the handlers with according assert info
    // Return FailBehaviour::Halt if at least one of them requested that

    const AssertInfo assertInfo(expr, fileName, lineNumber, formattedMessage, backtrace);

    bool halt = false;
    for (const Handler& handler : registredHandlers)
    {
        const FailBehaviour handlerFailBehaviour = handler(assertInfo);
        halt |= (handlerFailBehaviour == FailBehaviour::Halt);
    }

    return halt ? FailBehaviour::Halt : FailBehaviour::Continue;
}

