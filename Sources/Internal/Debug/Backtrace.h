#ifndef __DAVAENGINE_BACKTRACE_H__
#define __DAVAENGINE_BACKTRACE_H__

// clang-format off
#include "Base/BaseTypes.h"

namespace DAVA
{
namespace Debug
{

struct StackFrame
{
    StackFrame() = default;
    StackFrame(void* addr_, const char8* func_)
        : addr(addr_)
        , function(func_)
    {}
    StackFrame(void* addr_, String func_)
        : addr(addr_)
        , function(std::move(func_))
    {}

    void* addr;
    String function;
};

DAVA_NOINLINE Vector<StackFrame> GetBacktrace(size_t framesToCapture = -1);

// Convert backtrace to string
//  1st version uses already obtained backtrace and formats up to nframes items
//  2nd version internally gets backtrace and formats all items
String BacktraceToString(const Vector<StackFrame>& backtrace, size_t nframes = -1);
String BacktraceToString(size_t framesToCapture);

// Low level function to get stack frames and symbols
DAVA_NOINLINE size_t GetStackFrames(void* frames[], size_t framesToCapture);
String GetSymbolFromAddr(void* addr, bool demangle = true);
String DemangleSymbol(const char8* symbol);

} // namespace Debug
} // namespace DAVA

// clang-format on

#endif // __DAVAENGINE_BACKTRACE_H__
