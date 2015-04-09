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

#include "SymbolTable.h"

#include "Debug/DVAssert.h"

using namespace DAVA;

void SymbolTable::AddSymbol(DAVA::uint64 frameAddr, const DAVA::String& name)
{
    auto iterAddr = symbols.find(frameAddr);
    if (iterAddr == symbols.end())
    {
        symbols.emplace(frameAddr, name);
    }
    else
    {
        // TODO: remove debug check
        DVASSERT(GetSymbol(frameAddr) == name);
    }
}

void SymbolTable::AddBacktrace(DAVA::uint32 hash, const DAVA::uint64* frames, size_t maxFrameDepth)
{
    DVASSERT(frames != nullptr && maxFrameDepth > 0);

    auto iterBktrace = bktraces.find(hash);
    if (iterBktrace == bktraces.end())
    {
        Backtrace bktrace = CreateBacktrace(hash, frames, maxFrameDepth);
        bktraces.emplace(hash, std::move(bktrace));
    }
    else
    {
        // TODO: remove debug check
        Backtrace& bktraceCur = iterBktrace->second;
        Backtrace bktraceNew = CreateBacktrace(hash, frames, maxFrameDepth);

        DVASSERT(bktraceCur.frames.size() == bktraceNew.frames.size());
        DVASSERT(std::equal(bktraceCur.frames.begin(), bktraceCur.frames.end(), bktraceNew.frames.begin()));
    }
}

const DAVA::String& SymbolTable::GetSymbol(DAVA::uint64 frameAddr) const
{
    auto iterAddr = symbols.find(frameAddr);
    if (iterAddr != symbols.end())
    {
        return iterAddr->second;
    }
    static const String empty;
    return empty;
}

const DAVA::Vector<DAVA::uint64>& SymbolTable::GetFrames(DAVA::uint32 hash) const
{
    auto iterBktrace = bktraces.find(hash);
    DVASSERT(iterBktrace != bktraces.end());
    if (iterBktrace != bktraces.end())
    {
        const Backtrace& o = iterBktrace->second;
        return o.frames;
    }
    static const Vector<uint64> empty;
    return empty;
}

SymbolTable::Backtrace SymbolTable::CreateBacktrace(DAVA::uint32 hash, const DAVA::uint64* frames, size_t maxFrameDepth) const
{
    size_t nframes = GetUsefulFramesCount(frames, maxFrameDepth);
    DVASSERT(nframes > 0);

    Backtrace bktrace(hash);
    bktrace.frames.reserve(nframes);
    for (size_t i = 0;i < nframes;++i)
    {
        bktrace.frames.push_back(frames[i]);
    }
    return bktrace;
}

size_t SymbolTable::GetUsefulFramesCount(const DAVA::uint64* frames, size_t maxFrameDepth) const
{
    size_t n = 0;
    const uint64 INVALID_FRAME = 0xFFFF;
    for (;n < maxFrameDepth && frames[n] > INVALID_FRAME;++n)
    {
    }
    return n;
}
