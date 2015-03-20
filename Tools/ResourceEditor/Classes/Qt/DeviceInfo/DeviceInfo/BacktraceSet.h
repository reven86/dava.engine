#ifndef __BACKTRACESET_H__
#define __BACKTRACESET_H__

#include "Base/BaseTypes.h"
#include "MemoryManager/MemoryManagerTypes.h"

class BacktraceSet
{
public:
    using SymbolMapType = DAVA::UnorderedMap<DAVA::uint64, DAVA::String>;
    using BacktraceMapType = DAVA::UnorderedMap<DAVA::uint32, DAVA::MMBacktrace>;

    using SymbolMapConstIterator = SymbolMapType::const_iterator;
    using BacktraceMapConstIterator = BacktraceMapType::const_iterator;

public:
    void AddSymbol(DAVA::uint64 addr, const DAVA::String& symbol);
    void AddBacktrace(const DAVA::MMBacktrace& bktrace);

    bool SymbolsEmpty() const { return symbolMap.empty(); }
    bool BacktraceEmpty() const { return backtraceMap.empty(); }

    size_t SymbolsCount() const { return symbolMap.size(); }
    size_t BacktraceCount() const { return backtraceMap.size(); }

    const DAVA::char8* GetSymbol(DAVA::uint64 addr) const;
    const DAVA::MMBacktrace* Getbacktrace(DAVA::uint32 hash) const;

    //SymbolMapType::iterator SymbolsBegin() { return symbolMap.begin(); }
    //SymbolMapType::iterator SymbolsEnd() { return symbolMap.end(); }
    SymbolMapConstIterator SymbolsBegin() const { return symbolMap.cbegin(); }
    SymbolMapConstIterator SymbolsEnd() const { return symbolMap.cend(); }
    SymbolMapConstIterator SymbolsCBegin() const { return symbolMap.cbegin(); }
    SymbolMapConstIterator SymbolsCEnd() const { return symbolMap.cend(); }

    //BacktraceMapType::iterator BacktraceBegin() { return backtraceMap.begin(); }
    //BacktraceMapType::iterator BacktraceEnd() { return backtraceMap.end(); }
    BacktraceMapConstIterator BacktraceBegin() const { return backtraceMap.cbegin(); }
    BacktraceMapConstIterator BacktraceEnd() const { return backtraceMap.cend(); }
    BacktraceMapConstIterator BacktraceCBegin() const { return backtraceMap.cbegin(); }
    BacktraceMapConstIterator BacktraceCEnd() const { return backtraceMap.cend(); }

private:
    SymbolMapType symbolMap;
    BacktraceMapType backtraceMap;
};

#endif  // __BACKTRACESET_H__
