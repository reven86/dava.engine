#include "Debug/DVAssert.h"

#include "Qt/DeviceInfo/MemoryTool/BacktraceSymbolTable.h"

using namespace DAVA;

const uint64 INVALID_ADDRESS = 0xFFFF;

const DAVA::String* BacktraceSymbolTable::AddSymbol(uint64 stackAddr, const String& name)
{
    DVASSERT(name.empty() == false && stackAddr > INVALID_ADDRESS);

    // Check whether frame address and its associated name are already in map
    auto iterAddr = addrToNameMap.find(stackAddr);
    if (iterAddr == addrToNameMap.end())
    {
        // Check whether name is already in unique names set, if not place it there
        auto iterUniqueName = uniqueNames.find(name);
        if (iterUniqueName == uniqueNames.end())
        {
            iterUniqueName = uniqueNames.emplace(name).first;
        }

        // Associate name with frame address
        const String* namePointer = &*iterUniqueName;
        iterAddr = addrToNameMap.emplace(stackAddr, namePointer).first;
    }
    else
    {   // TODO: remove, for debug purpose
        const String& nameInMap = *iterAddr->second;
        DVASSERT(nameInMap == name);
    }
    return iterAddr->second;
}

void BacktraceSymbolTable::AddBacktrace(uint32 hash, const uint64* stackFrames, size_t stackDepth)
{
    DVASSERT(stackFrames != nullptr && stackDepth > 0);

    auto iter = bktraceMap.find(hash);
    if (iter == bktraceMap.end())
    {
        bktraceMap.emplace(hash, ResolveFrameNames(stackFrames, stackDepth));
    }
    else
    {   // TODO: remove, for debug purpose
        const Vector<const String*> y = iter->second;
        Vector<const String*> x = ResolveFrameNames(stackFrames, stackDepth);
        if (x != y)
        {
            Logger::Error("[BacktraceSymbolTable] backtrace hash collision: %u", hash);
        }
    }
}

const String* BacktraceSymbolTable::GetSymbol(uint64 stackAddr) const
{
    auto iter = addrToNameMap.find(stackAddr);
    return iter != addrToNameMap.end() ? iter->second : nullptr;
}

const Vector<const String*>* BacktraceSymbolTable::GetBacktraceSymbols(uint32 bktraceHash) const
{
    auto iter = bktraceMap.find(bktraceHash);
    return iter != bktraceMap.end() ? &iter->second : nullptr;
}

const String* BacktraceSymbolTable::GenerateAndAddSymbol(uint64 stackAddr)
{
    const size_t NAMEBUF_LEN = 32;
    char8 namebuf[NAMEBUF_LEN];
    Snprintf(namebuf, NAMEBUF_LEN, "#%08llX", stackAddr);
    return AddSymbol(stackAddr, String(namebuf));
}

size_t BacktraceSymbolTable::GetValidFramesCount(const uint64* stackFrames, size_t stackDepth) const
{
    auto iter = std::find_if(stackFrames, stackFrames + stackDepth, [](const uint64 frame) -> bool { return frame <= INVALID_ADDRESS; });
    return std::distance(stackFrames, iter);
}

Vector<const String*> BacktraceSymbolTable::ResolveFrameNames(const uint64* stackFrames, size_t stackDepth)
{
    size_t n = GetValidFramesCount(stackFrames, stackDepth);
    DVASSERT(n > 0);

    Vector<const String*> names;
    names.reserve(n);
    for (size_t i = 0;i < n;++i)
    {
        const DAVA::String* name = GetSymbol(stackFrames[i]);
        if (nullptr == name)
        {
            name = GenerateAndAddSymbol(stackFrames[i]);
        }
        names.push_back(name);
    }
    return names;
}
