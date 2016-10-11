#pragma once

#include "Base/BaseTypes.h"
#include "Debug/TraceEvent.h"
#include <iosfwd>

#define PROFILER_CPU_ENABLED 1

namespace DAVA
{
template <class T>
class ProfilerRingArray;

class ProfilerCPU
{
public:
    struct Counter;
    using CounterArray = ProfilerRingArray<ProfilerCPU::Counter>;

    class ScopedCounter
    {
    public:
        ScopedCounter(const char* counterName, ProfilerCPU* profiler);
        ~ScopedCounter();

    private:
        uint64* endTime = nullptr;
        ProfilerCPU* profiler;
    };

    static const int32 NO_SNAPSHOT_ID = -1; //use to dump current trace
    static ProfilerCPU* const globalProfiler;

    ProfilerCPU(uint32 countersCount = 2048);
    ~ProfilerCPU();

    void Start();
    void Stop();
    bool IsStarted();

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

#if PROFILER_CPU_ENABLED

#define DAVA_PROFILER_CPU_SCOPE(counter_name) DAVA::ProfilerCPU::ScopedCounter time_profiler_scope_counter(counter_name, DAVA::ProfilerCPU::globalProfiler);

#else

#define DAVA_PROFILER_CPU_SCOPE(counter_name)

#endif