#pragma once

#include "Base/BaseTypes.h"
#include "Debug/TraceEvent.h"
#include <iosfwd>

#define CPU_PROFILER_ENABLED 1

namespace DAVA
{
template <class T>
class ProfilerRingArray;

class CPUProfiler
{
public:
    struct Counter;
    using CounterArray = ProfilerRingArray<CPUProfiler::Counter>;

    class ScopedCounter
    {
    public:
        ScopedCounter(const char* counterName, CPUProfiler* profiler);
        ~ScopedCounter();

    private:
        uint64* endTime = nullptr;
        CPUProfiler* profiler;
    };

    static const int32 NO_SNAPSHOT_ID = -1; //use to dump current trace
    static CPUProfiler* const globalProfiler;

    CPUProfiler(uint32 countersCount = 2048);
    ~CPUProfiler();

    void Start();
    void Stop();

    uint64 GetLastCounterTime(const char* counterName);

    int32 MakeSnapshot();
    void DeleteSnapshot(int32 snapshot);
    void DeleteSnapshots();

    void DumpJSON(std::ostream& stream, int32 snapshot = NO_SNAPSHOT_ID);
    void DumpLast(const char* counterName, uint32 counterCount, std::ostream& stream, int32 snapshot = NO_SNAPSHOT_ID);
    void DumpAverage(const char* counterName, uint32 counterCount, std::ostream& stream, int32 snapshot = NO_SNAPSHOT_ID);

    Vector<TraceEvent> GetTrace(int32 snapshot = NO_SNAPSHOT_ID);
    Vector<TraceEvent> GetTrace(const char* counterName, uint32 counterCount = 1, int32 snapshot = NO_SNAPSHOT_ID);

protected:
    CounterArray* GetCounterArray(int32 snapshot);

    CounterArray* counters;
    Vector<CounterArray*> snapshots;
    bool started = false;

    friend class ScopedCounter;
};

} //ns DAVA

#if CPU_PROFILER_ENABLED

#define DAVA_CPU_PROFILER_SCOPE(counter_name) DAVA::CPUProfiler::ScopedCounter time_profiler_scope_counter(counter_name, DAVA::CPUProfiler::globalProfiler);

#else

#define DAVA_CPU_PROFILER_SCOPE(counter_name)

#endif