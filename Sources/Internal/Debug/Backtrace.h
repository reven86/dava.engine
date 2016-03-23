/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#ifndef __DAVAENGINE_BACKTRACE_H__
#define __DAVAENGINE_BACKTRACE_H__

// clang-format off
#include "Base/BaseTypes.h"
#include "Logger/Logger.h"

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

// Prints backtrace to log using specified log level
//  1st version uses already obtained backtrace
//  2nd version internally gets backtrace and logs all items
void BacktraceToLog(const Vector<StackFrame>& backtrace, Logger::eLogLevel ll = Logger::LEVEL_ERROR);
void BacktraceToLog(size_t framesToCapture = -1, Logger::eLogLevel ll = Logger::LEVEL_ERROR);

// Low level function to get stack frames and symbols
DAVA_NOINLINE size_t GetStackFrames(void* frames[], size_t framesToCapture);
String GetSymbolFromAddr(void* addr, bool demangle = true);
String DemangleSymbol(const char8* symbol);

} // namespace Debug
} // namespace DAVA

#endif // __DAVAENGINE_BACKTRACE_H__
