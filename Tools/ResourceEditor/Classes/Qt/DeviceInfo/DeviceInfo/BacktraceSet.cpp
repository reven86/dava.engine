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
    auto i = backtraceMap.emplace(std::make_pair(bktrace.hash, bktrace));
    if (i.second)
    {
        MMBacktrace& o = (*i.first).second;
        o.depth = static_cast<uint32>(ComputeDepth(bktrace));
    }
}

void BacktraceSet::Rebuild()
{
    for (auto& x : symbolMap)
    {
        uniqueNames.emplace(x.second);
    }

    for (auto& x : backtraceMap)
    {
        MMBacktrace& o = x.second;
        for (size_t i = 0, n = o.depth;i < n;++i)
        {
            auto sym = symbolMap.find(o.frames[i]);
            if (sym != symbolMap.end())
            {
                auto uniq = uniqueNames.find(sym->second);
                o.frames[i] = reinterpret_cast<uint64>(uniq->c_str());
            }
            else
            {
                char8 buf[32];
                Snprintf(buf, COUNT_OF(buf), "%08llX", o.frames[i]);
                auto k = uniqueNames.emplace(String(buf)).first;
                o.frames[i] = reinterpret_cast<uint64>(k->c_str());
            }
        }
    }

    /*size_t k = 0;
    for (auto i = backtraceMap.begin(), e = backtraceMap.end();i != e;++i)
    {
        MMBacktrace& o = i->second;
        for (size_t i = 0, n = o.depth;i < n;++i)
        {
            const char8* s = GetSymbol(o.frames[i]);
            if (s == nullptr)
            {
                char8 buf[32];
                Snprintf(buf, COUNT_OF(buf), "%08llX", o.frames[i]);
                AddSymbol(o.frames[i], String(buf));
                k += 1;
            }
        }
    }
    for (auto i = backtraceMap.begin(), e = backtraceMap.end();i != e;++i)
    {
        MMBacktrace& o = i->second;
        for (size_t i = 0, n = o.depth;i < n;++i)
        {
            const char8* s = GetSymbol(o.frames[i]);
            Q_ASSERT(s != nullptr);
            o.frames[i] = reinterpret_cast<uint64>(s);
        }
    }*/
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

size_t BacktraceSet::ComputeDepth(const DAVA::MMBacktrace& o) const
{
    size_t n = 0;
    while (n < MMConst::BACKTRACE_DEPTH && o.frames[n] > 0xFFFF)
        n += 1;
    return n;
}
