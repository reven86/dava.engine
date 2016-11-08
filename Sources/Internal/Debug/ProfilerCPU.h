#pragma once

#include "Base/BaseTypes.h"
#include "Debug/TraceEvent.h"
#include <iosfwd>

#define PROFILER_CPU_ENABLED 1

namespace DAVA
{
template <class T>
class ProfilerRingArray;

/**
    \ingroup profilers
    \brief   Profiler allows to measure execution time of code-blocks from any thread.
    \details Profiler allows to measure execution time of code-blocks from any thread. It's cheap in performance. If profiler not started - it's almost free.
             To use this profiler, at first, you should place counters in interesting code blocks using set of DAVA_PROFILER_CPU_SCOPE defines listed below.
             Than you just start profiler. After that you can dump counted info or build trace to view it in Chromium Trace Viewer.
            
             Any counter has string-name that passed to define and displayed in dump or trace. Time-measuring occurs in microseconds.

             Profiler used ring array for counters so you limited by count passed to ctor. If necessary store counters data to use it later you can use snapshots.
             Snapshot - it just a copy of internal ring buffer. So to make snapshot you should stop profiler because it can be used by any thread.
             After snapshot was made you can dump counted info or build JSON-trace from it. Remember, that dump or building trace is more expensive in performance than making snapshot.
            
             Engine has own global profiler. You can access it through static field 'ProfilerCPU::globalProfiler'. 
             All counters names placed over the engine are listed in 'ProfilerCPUMarkerName' namespace (ProfilerMarkerNames.h).
             You can add counters to global engine profiler or you can create own profiler and use it separately. 

             The following define are used to place counters:
              - DAVA_PROFILER_CPU_SCOPE(name)                                           -- Add counter with to global engine profiler.
              - DAVA_PROFILER_CPU_SCOPE_WITH_FRAME_INDEX(name, index)                   -- Mark counter by frame index and add to global engine profiler.
              - DAVA_PROFILER_CPU_SCOPE_CUSTOM(name, profiler)                          -- Add counter with to custom profiler.
              - DAVA_PROFILER_CPU_SCOPE_CUSTOM_WITH_FRAME_INDEX(name, profiler, index)  -- Mark counter by frame index and add to custom profiler.

             Frame index from counter in a subsequent can be viewed in TraceEvent args. Name on argument is 'TRACE_ARG_FRAME'. For more information about trace events arguments see 'TraceEvent'.

             Dump and trace considers hierarchy of counters. It's means that if counter A started after counter B and completed before B is completed (counters A and B placed in the same thread),
             in this case counter A became child of B. And dump of any counter include all child counters. So, dump represents as tree like call-tree. For example:
               \code
                 ================================================================
                 Counter0 [120 us | x1]
                   Counter1 [60 us | x1]
                     Counter4 [10 us | x4]
                   Counter2 [15 us | x3]
                   Counter3 [15 us | x1]
                 ================================================================
               \endcode
*/
class ProfilerCPU
{
public:
    static const FastName TRACE_ARG_FRAME; ///< Name of frame index argument of generated TraceEvent

    struct Counter;
    using CounterArray = ProfilerRingArray<ProfilerCPU::Counter>;

    /**
        Scoped counter to measure executing time of code block.
        Use DAVA_PROFILER_CPU_SCOPE defines instead of manual object creation.
    */
    class ScopedCounter
    {
    public:
        ScopedCounter(const char* counterName, ProfilerCPU* profiler, uint32 frame = 0U);
        ~ScopedCounter();

    private:
        uint64* endTime = nullptr;
        ProfilerCPU* profiler;
    };

    static const int32 NO_SNAPSHOT_ID = -1; ///< Value used to dump or build trace from current counters array
    static ProfilerCPU* const globalProfiler; ///< Global Engine Profiler

    ProfilerCPU(uint32 countersCount = 2048);
    ~ProfilerCPU();

    /**
        Start time measuring
    */
    void Start();

    /**
        Stop time measuring
    */
    void Stop();

    /**
        \return Is time measuring started
    */
    bool IsStarted();

    /**
        Looking by name last complete counter and return it duration in microseconds
        \return last counter duration
    */
    uint64 GetLastCounterTime(const char* counterName);

    /**
        Make snapshot. Snapshot is just a copy of counters array.
        You should stop profiler before making snapshot,
        \return snapshot ID
    */
    int32 MakeSnapshot();

    /**
        Delete snapshot by ID
        \param[in] snapshot Snapshot ID
    */
    void DeleteSnapshot(int32 snapshot);

    /**
        Delete all created snapshots
    */
    void DeleteSnapshots();

    /**
        Looking for a certain amount of last completed counters by name and dump it to stream.
        You can dump output from snapshot or from current counters array counters. In the second case you should stop profiler.

        \param[in] counterName Name of counter
        \param[in] counterCount Count of looking counters
        \param[out] stream Stream which output will be written
        \param[in] snapshot Snapshot ID. Can be 'NO_SNAPSHOT_ID' for dump from current counters array
    */
    void DumpLast(const char* counterName, uint32 counterCount, std::ostream& stream, int32 snapshot = NO_SNAPSHOT_ID);

    /**
        Looking for a certain amount of last completed counters by name and dump it average durations to stream considering hierarchy.
        You can dump output from snapshot or from current counters array. In the second case you should stop profiler.

        \param[in] counterName Name of counter
        \param[in] counterCount Count of looking counters
        \param[out] stream Stream which output will be written
        \param[in] snapshot Snapshot ID. Can be 'NO_SNAPSHOT_ID' for dump from current counters array counters
    */
    void DumpAverage(const char* counterName, uint32 counterCount, std::ostream& stream, int32 snapshot = NO_SNAPSHOT_ID);

    /**
        Build trace of all available counters from snapshot or internal counters array. 
        Trace can be dumped to JSON Chromium Trace Viewer format.

        \param[in] snapshot Snapshot ID. Can be 'NO_SNAPSHOT_ID' for dump from current counters array counters
        \return Vector of trace events
    */
    Vector<TraceEvent> GetTrace(int32 snapshot = NO_SNAPSHOT_ID);

    /**
        Looking last completed counter by name and frame index. Frame index is desired so it will used counter marked with <= index for build trace.
        If pass frame index equals 0, search-criteria by index will be ignored.

        Gotten trace can be dumped to JSON Chromium Trace Viewer format.

        \param[in] counterName Name of counter
        \param[in] desiredFrameIndex Desired frame index. Pass zero to ignore this search criteria
        \param[in] snapshot Snapshot ID. Can be 'NO_SNAPSHOT_ID' for dump from current counters array counters
        \return Vector of trace events
    */
    Vector<TraceEvent> GetTrace(const char* counterName, uint32 desiredFrameIndex = 0, int32 snapshot = NO_SNAPSHOT_ID);

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
#define DAVA_PROFILER_CPU_SCOPE_WITH_FRAME_INDEX(counter_name, index) DAVA::ProfilerCPU::ScopedCounter time_profiler_scope_counter(counter_name, DAVA::ProfilerCPU::globalProfiler, index);

#define DAVA_PROFILER_CPU_SCOPE_CUSTOM(counter_name, profiler) DAVA::ProfilerCPU::ScopedCounter time_profiler_scope_counter_custom(counter_name, profiler);
#define DAVA_PROFILER_CPU_SCOPE_CUSTOM_WITH_FRAME_INDEX(counter_name, profiler, index) DAVA::ProfilerCPU::ScopedCounter time_profiler_scope_counter_custom(counter_name, profiler, index);

#else

#define DAVA_PROFILER_CPU_SCOPE(counter_name)
#define DAVA_PROFILER_CPU_SCOPE_WITH_FRAME_INDEX(counter_name, index)

#define DAVA_PROFILER_CPU_SCOPE_CUSTOM(counter_name, profiler)
#define DAVA_PROFILER_CPU_SCOPE_CUSTOM_WITH_FRAME_INDEX(counter_name, profiler, index)

#endif