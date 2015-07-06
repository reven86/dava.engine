#ifndef __BACKTRACESYMBOLTABLE_H_
#define __BACKTRACESYMBOLTABLE_H_

#include "Base/BaseTypes.h"
#include "MemoryManager/MemoryManagerTypes.h"

class BacktraceSymbolTable
{
public:
    struct Backtrace
    {
        Backtrace(DAVA::uint32 aHash) : hash(aHash) {}
        Backtrace(const Backtrace&) = delete;
        Backtrace& operator=(const Backtrace&) = delete;

        Backtrace(Backtrace&& o) : hash(o.hash) , frameNames(std::move(o.frameNames))
        {}
        Backtrace& operator = (Backtrace&&) = delete;
        DAVA::uint32 hash;
        DAVA::Vector<const char*> frameNames;
    };

public:
    void AddSymbol(DAVA::uint64 frameAddr, const DAVA::String& name);
    void AddSymbol(DAVA::uint64 frameAddr);
    void AddBacktrace(DAVA::uint32 hash, const DAVA::uint64* frames, size_t maxFrameDepth);

    const DAVA::String& GetSymbol(DAVA::uint64 frameAddr) const;
    const DAVA::Vector<const char*>& GetFrames(DAVA::uint32 hash) const;

    template<typename F>
    void IterateOverSymbols(F fn) const;
    template<typename F>
    void IterateOverBacktraces(F fn) const;

private:
    Backtrace CreateBacktrace(DAVA::uint32 hash, const DAVA::uint64* frames, size_t maxFrameDepth);
    size_t GetUsefulFramesCount(const DAVA::uint64* frames, size_t maxFrameDepth) const;

private:
    DAVA::UnorderedMap<DAVA::uint64, DAVA::uint32> addrToNameMap;
    DAVA::UnorderedMap<DAVA::uint32, DAVA::String> uniqueNames;

    DAVA::UnorderedMap<DAVA::uint32, Backtrace> bktraces;
};

//////////////////////////////////////////////////////////////////////////
template<typename F>
void BacktraceSymbolTable::IterateOverSymbols(F fn) const
{
    for (auto& x : uniqueNames)
    {
        fn(x.second.c_str());
    }
}

template<typename F>
void BacktraceSymbolTable::IterateOverBacktraces(F fn) const
{
    for (auto& x : bktraces)
    {
        fn(x.first, x.second.frameNames);
    }
}

#endif  // __BACKTRACESYMBOLTABLE_H_
