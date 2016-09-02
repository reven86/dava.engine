#pragma once

#include "Base/BaseTypes.h"
#include "Base/Hash.h"
#include <iosfwd>

using DAVA::int32;
using DAVA::uint32;
using DAVA::uint64;

#define PROFILER_ENABLED 1

namespace DAVA
{
namespace Profiler
{
static const int32 NO_SNAPSHOT_ID = -1; //use to dump current trace

void Start();
void Stop();

uint64 GetLastCounterTime(const char* counterName);

int32 MakeSnapshot();
void DeleteSnapshot(int32 snapshot);
void DeleteSnapshots();

void DumpJSON(std::ostream& stream, int32 snapshot = NO_SNAPSHOT_ID);
void DumpLast(const char* counterName, uint32 counterCount, std::ostream& stream, int32 snapshot = NO_SNAPSHOT_ID);
void DumpAverage(const char* counterName, uint32 counterCount, std::ostream& stream, int32 snapshot = NO_SNAPSHOT_ID);

class ScopedCounter
{
public:
    ScopedCounter(const char* counterName, uint32 counterNameID);
    ~ScopedCounter();

private:
    uint64* endTime = nullptr;
};

} //ns Profiler

} //ns DAVA

#if PROFILER_ENABLED

#define PROFILER_TIMING(counter_name) DAVA::Profiler::ScopedCounter time_profiler_scope_counter(counter_name, DV_HASH(counter_name));

#else

#define PROFILER_TIMING(counter_name)

#endif