#pragma once
#include <stdexcept>
#include "Base/BaseTypes.h"
#include "Debug/Backtrace.h"

namespace DAVA
{
class Exception : public ::std::runtime_error
{
public:
    Exception(const String& message, const char* file_, size_t line_)
        : ::std::runtime_error(message)
        , file(file_)
        , line(line_)
        , callstack(Debug::GetBacktrace())
    {
    }

    String file;
    size_t line;
    Vector<Debug::StackFrame> callstack;
};

} // namespace DAVA

#define DAVA_THROW(e, ...) throw e(__VA_ARGS__, __FILE__, __LINE__)
