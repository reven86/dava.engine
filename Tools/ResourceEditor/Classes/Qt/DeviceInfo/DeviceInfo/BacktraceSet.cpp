#include <QDebug>

#include "BacktraceSet.h"

using namespace DAVA;

void BacktraceSet::AddSymbol(DAVA::uint64 addr, const DAVA::String& symbol)
{
    Q_ASSERT(symbolMap.find(addr) == symbolMap.cend());
    symbolMap.emplace(std::make_pair(addr, symbol));
}

void BacktraceSet::AddBacktrace(const DAVA::MMBacktrace& bktrace)
{
    Q_ASSERT(backtraceMap.find(bktrace.hash) == backtraceMap.cend());
    backtraceMap.emplace(std::make_pair(bktrace.hash, bktrace));
}

const char8* BacktraceSet::GetSymbol(DAVA::uint64 addr) const
{
    auto i = symbolMap.find(addr);
    if (i != symbolMap.cend())
    {
        return i->second.c_str();
    }
    return nullptr;
}

const DAVA::MMBacktrace* BacktraceSet::Getbacktrace(DAVA::uint32 hash) const
{
    auto i = backtraceMap.find(hash);
    if (i != backtraceMap.cend())
    {
        return &i->second;
    }
    return nullptr;
}
