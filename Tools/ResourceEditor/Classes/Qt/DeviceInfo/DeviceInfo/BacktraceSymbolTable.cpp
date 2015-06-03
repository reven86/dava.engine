#include "Base/Hash.h"
#include "Debug/DVAssert.h"

#include "BacktraceSymbolTable.h"

using namespace DAVA;

void BacktraceSymbolTable::AddSymbol(DAVA::uint64 frameAddr, const DAVA::String& name)
{
    auto iterAddr = addrToNameMap.find(frameAddr);
    if (iterAddr == addrToNameMap.end())
    {
        uint32 nameHash = HashValue_N(name.c_str(), static_cast<size_t>(name.length()));
        auto iterName = uniqueNames.find(nameHash);
        if (iterName == uniqueNames.end())
        {
            uniqueNames.emplace(nameHash, name);
        }
        addrToNameMap.emplace(frameAddr, nameHash);
    }
}

void BacktraceSymbolTable::AddSymbol(DAVA::uint64 frameAddr)
{
    auto iterAddr = addrToNameMap.find(frameAddr);
    if (iterAddr == addrToNameMap.end())
    {
        char name[32];
        Snprintf(name, COUNT_OF(name), "%08llX", frameAddr);

        uint32 nameHash = HashValue_N(name, strlen(name));
        auto iterName = uniqueNames.find(nameHash);
        if (iterName == uniqueNames.end())
        {
            uniqueNames.emplace(nameHash, name);
        }
        addrToNameMap.emplace(frameAddr, nameHash);
    }
}

void BacktraceSymbolTable::AddBacktrace(DAVA::uint32 hash, const DAVA::uint64* frames, size_t maxFrameDepth)
{
    DVASSERT(frames != nullptr && maxFrameDepth > 0);

    auto iterBktrace = bktraces.find(hash);
    if (iterBktrace == bktraces.end())
    {
        bktraces.emplace(hash, CreateBacktrace(hash, frames, maxFrameDepth));
    }
}

const DAVA::String& BacktraceSymbolTable::GetSymbol(DAVA::uint64 frameAddr) const
{
    auto iterAddr = addrToNameMap.find(frameAddr);
    if (iterAddr != addrToNameMap.end())
    {
        const uint32 nameHash = iterAddr->second;
        auto iterName = uniqueNames.find(nameHash);
        if (iterName != uniqueNames.end())
        {
            return iterName->second;
        }
    }
    static const String empty;
    return empty;
}

const DAVA::Vector<const char*>& BacktraceSymbolTable::GetFrames(DAVA::uint32 hash) const
{
    auto iterBktrace = bktraces.find(hash);
    DVASSERT(iterBktrace != bktraces.end());
    if (iterBktrace != bktraces.end())
    {
        const Backtrace& o = iterBktrace->second;
        return o.frameNames;
    }
    static const Vector<const char*> empty;
    return empty;
}

BacktraceSymbolTable::Backtrace BacktraceSymbolTable::CreateBacktrace(DAVA::uint32 hash, const DAVA::uint64* frames, size_t maxFrameDepth)
{
    size_t nframes = GetUsefulFramesCount(frames, maxFrameDepth);
    DVASSERT(nframes > 0);

    Backtrace bktrace(hash);
    bktrace.frameNames.reserve(nframes);
    for (size_t i = 0;i < nframes;++i)
    {
        const char* s = GetSymbol(frames[i]).c_str();
        if (s[0] == '\0')
        {
            char buf[32];
            Snprintf(buf, COUNT_OF(buf), "#_%08llX", frames[i]);
            AddSymbol(frames[i], buf);
            s = GetSymbol(frames[i]).c_str();
        }
        bktrace.frameNames.push_back(s);
    }
    return bktrace;
}

size_t BacktraceSymbolTable::GetUsefulFramesCount(const DAVA::uint64* frames, size_t maxFrameDepth) const
{
    size_t n = 0;
    const uint64 INVALID_FRAME = 0xFFFF;
    for (;n < maxFrameDepth && frames[n] > INVALID_FRAME;++n)
    {}
    return n;
}
