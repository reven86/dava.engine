#pragma once

#include "Base/BaseTypes.h"
using DAVA::int32;
using DAVA::uint32;
using DAVA::uint64;

#define PROFILER_ENABLED 1


namespace DAVA
{
class File;

namespace Profiler
{
void Start();
void Stop();

uint64 GetLastCounterTime(const char* counterName);

int32 MakeSnapshot();
void DeleteSnapshots();

void DumpJSON(File* file, int32 snapshot = -1);
void DumpLast(const char* counterName, uint32 counterCount, File* file = nullptr, int32 snapshot = -1);
void DumpAverage(const char* counterName, uint32 counterCount, File* file = nullptr, int32 snapshot = -1);

class ScopedCounter
{
public:
    ScopedCounter(const char* counterName);
    ~ScopedCounter();

private:
    uint64* endTime = nullptr;
};

} //ns Profiler

} //ns DAVA

#if PROFILER_ENABLED

#define PROFILER_TIMING(counter_name) DAVA::Profiler::ScopedCounter time_profiler_scope_counter(counter_name);

#else

#define PROFILER_TIMING(counter_name)

#endif