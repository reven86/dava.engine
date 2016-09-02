#pragma once

#include "Base/BaseTypes.h"
#include "Base/Hash.h"
#include <iosfwd>

#define PROFILER_ENABLED 1

namespace DAVA
{
template <class T>
class ProfilerRingArray;

namespace Profiler
{
struct TimeCounter;
using CounterArray = ProfilerRingArray<Profiler::TimeCounter>;

class TimeProfiler
{
public:
    class ScopedTimeCounter
    {
    public:
        ScopedTimeCounter(const char* counterName, uint32 counterNameID, TimeProfiler* profiler);
        ~ScopedTimeCounter();

    private:
        uint64* endTime = nullptr;
        TimeProfiler* profiler;
    };

    static const int32 NO_SNAPSHOT_ID = -1; //use to dump current trace
    static TimeProfiler* const GlobalProfiler;

    TimeProfiler(uint32 countersCount = 2048);
    ~TimeProfiler();

    void Start();
    void Stop();

    uint64 GetLastCounterTime(const char* counterName);

    int32 MakeSnapshot();
    void DeleteSnapshot(int32 snapshot);
    void DeleteSnapshots();

    void DumpJSON(std::ostream& stream, int32 snapshot = NO_SNAPSHOT_ID);
    void DumpLast(const char* counterName, uint32 counterCount, std::ostream& stream, int32 snapshot = NO_SNAPSHOT_ID);
    void DumpAverage(const char* counterName, uint32 counterCount, std::ostream& stream, int32 snapshot = NO_SNAPSHOT_ID);

protected:
    CounterArray* GetCounterArray(int32 snapshot);

    CounterArray* counters;
    Vector<CounterArray*> snapshots;
    bool started = false;

    friend class ScopedTimeCounter;
};

} //ns Profiler

} //ns DAVA

#if PROFILER_ENABLED

#define PROFILER_TIMING(counter_name) DAVA::Profiler::TimeProfiler::ScopedTimeCounter time_profiler_scope_counter(counter_name, DV_HASH(counter_name), DAVA::Profiler::TimeProfiler::GlobalProfiler);
#define PROFILER_TIMING_CUSTOM(counter_name, time_profiler) DAVA::Profiler::TimeProfiler::ScopedTimeCounter time_profiler_scope_counter(counter_name, DV_HASH(counter_name), time_profiler);

#else

#define PROFILER_TIMING(counter_name)
#define PROFILER_TIMING_CUSTOM(counter_name, time_profiler)

#endif