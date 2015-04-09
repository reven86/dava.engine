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

#ifndef __SYMBOLTABLE_H__
#define __SYMBOLTABLE_H__

#include "Base/BaseTypes.h"

class SymbolTable
{
private:
    struct Backtrace
    {
        Backtrace(DAVA::uint32 aHash) : hash(aHash) {}
        Backtrace(const Backtrace&) = delete;
        Backtrace& operator=(const Backtrace&) = delete;

        Backtrace(Backtrace&& o) : hash(o.hash), frames(std::move(o.frames))
        {}
        Backtrace& operator = (Backtrace&&) = delete;

        DAVA::uint32 hash;
        DAVA::Vector<DAVA::uint64> frames;
    };

public:
    void AddSymbol(DAVA::uint64 frameAddr, const DAVA::String& name);
    void AddBacktrace(DAVA::uint32 hash, const DAVA::uint64* frames, size_t maxFrameDepth);

    const DAVA::String& GetSymbol(DAVA::uint64 frameAddr) const;
    const DAVA::Vector<DAVA::uint64>& GetFrames(DAVA::uint32 hash) const;

    size_t BkTraceCount() const { return bktraces.size();  }
    template<typename F>
    void IterateOverBacktraces(F fn) const
    {
        for (auto& x : bktraces)
        {
            fn(x.first, x.second.frames);
        }
    }

private:
    Backtrace CreateBacktrace(DAVA::uint32 hash, const DAVA::uint64* frames, size_t maxFrameDepth) const;
    size_t GetUsefulFramesCount(const DAVA::uint64* frames, size_t maxFrameDepth) const;

private:
    DAVA::Map<DAVA::uint64, DAVA::String> symbols;
    DAVA::Map<DAVA::uint32, Backtrace> bktraces;
};

#endif  // __SYMBOLTABLE_H__
