#ifndef __BACKTRACESYMBOLTABLE_H_
#define __BACKTRACESYMBOLTABLE_H_

#include "Base/BaseTypes.h"
#include "MemoryManager/MemoryManagerTypes.h"

class BacktraceSymbolTable
{
public:
    BacktraceSymbolTable() = default;
    ~BacktraceSymbolTable() = default;

    const DAVA::String* AddSymbol(DAVA::uint64 stackAddr, const DAVA::String& name);
    void AddBacktrace(DAVA::uint32 hash, const DAVA::uint64* stackFrames, size_t stackDepth);

    const DAVA::String* GetSymbol(DAVA::uint64 stackAddr) const;
    const DAVA::Vector<const DAVA::String*>* GetBacktraceSymbols(DAVA::uint32 bktraceHash) const;

    size_t SymbolCount() const;
    size_t BacktraceCount() const;

    template<typename F>
    void IterateOverSymbols(F fn) const;
    template<typename F>
    void IterateOverBacktraces(F fn) const;

private:
    const DAVA::String* GenerateAndAddSymbol(DAVA::uint64 stackAddr);

    size_t GetValidFramesCount(const DAVA::uint64* stackFrames, size_t stackDepth) const;
    DAVA::Vector<const DAVA::String*> ResolveFrameNames(const DAVA::uint64* stackFrames, size_t stackDepth);

private:
    DAVA::UnorderedSet<DAVA::String> uniqueNames;
    DAVA::UnorderedMap<DAVA::uint64, const DAVA::String*> addrToNameMap;
    DAVA::UnorderedMap<DAVA::uint32, DAVA::Vector<const DAVA::String*>> bktraceMap; // Keep names instead of addresses
};

//////////////////////////////////////////////////////////////////////////
inline size_t BacktraceSymbolTable::SymbolCount() const
{
    return uniqueNames.size();
}

inline size_t BacktraceSymbolTable::BacktraceCount() const
{
    return bktraceMap.size();
}

template<typename F>
void BacktraceSymbolTable::IterateOverSymbols(F fn) const
{
    std::for_each(uniqueNames.cbegin(), uniqueNames.cend(), fn);
}

template<typename F>
void BacktraceSymbolTable::IterateOverBacktraces(F fn) const
{
    std::for_each(bktraceMap.cbegin(), bktraceMap.cend(), [fn](const std::pair<DAVA::uint32, DAVA::Vector<const DAVA::String*>>& x) { fn(x.first, x.second);});
}

#endif  // __BACKTRACESYMBOLTABLE_H_
