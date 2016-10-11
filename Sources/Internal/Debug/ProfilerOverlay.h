#pragma once

#include "Base/BaseTypes.h"
#include "Base/FastNameMap.h"
#include "Debug/Private/RingArray.h"

namespace DAVA
{
class ProfilerCPU;
class ProfilerGPU;
class ProfilerOverlay
{
public:
    static const uint32 TRACE_HISTORY_SIZE = 10;
    static ProfilerOverlay* const globalProfilerOverlay;

    ProfilerOverlay(ProfilerCPU* cpuProfiler, const char* cpuCounterName, ProfilerGPU* gpuProfiler, const Vector<FastName>& interestEvents = Vector<FastName>());

    void Enable();
    void Disable();

    void OnFrameEnd(); //should be called before rhi::Present();

    void ClearInterestEvents();
    void AddInterestEvent(const FastName& name);
    Vector<FastName> GetAvalibleEventsNames() const;

    void SetTraceHistoryOffset(uint32 offset);
    uint32 GetTraceHistoryOffset() const;

    void SetCPUProfiler(ProfilerCPU* profiler, const char* counterName);
    void SetGPUProfiler(ProfilerGPU* profiler);

protected:
    static const uint32 EVENT_HISTORY_LENGTH = 300;

    struct HistoryInstance
    {
        uint64 accurate = 0;
        float32 filtered = 0.f;
    };
    using HistoryArray = RingArray<HistoryInstance>;

    struct EventHistory
    {
        HistoryArray values;
        uint32 updatesCount;
    };

    struct TraceData
    {
        struct TraceRect
        {
            uint64 start;
            uint64 duration;
            uint32 color;
            int32 depth;
        };

        Vector<TraceRect> rects;
        Set<FastName> names;
        uint32 frameIndex = 0;
        uint64 minTimestamp = uint64(-1);
        uint64 maxTimestamp = 0;
    };

    struct FrameTrace
    {
        Vector<TraceEvent> trace;
        uint32 frameIndex;
    };

    void Update();
    void ProcessEventsTrace(const Vector<TraceEvent>& events, uint32 frameIndex, TraceData* trace);

    void Draw();
    void DrawTrace(const TraceData& trace, const char* traceHeader, const Rect2i& rect);
    void DrawHistory(const HistoryArray& history, const FastName& name, const Rect2i& rect);

    int32 GetEnoughRectHeight(const TraceData& trace);

    FastNameMap<EventHistory> eventsHistory = FastNameMap<EventHistory>(128, EventHistory({ HistoryArray(EVENT_HISTORY_LENGTH), 0 }));
    FastNameMap<uint32> eventsColor;

    Vector<FastName> interestEventsNames;
    uint32 maxEventNameLen = 0;

    RingArray<TraceData> GPUTraceData = RingArray<TraceData>(std::size_t(TRACE_HISTORY_SIZE));
    RingArray<TraceData> CPUTraceData = RingArray<TraceData>(std::size_t(TRACE_HISTORY_SIZE));
    List<FrameTrace> CPUFrameTraces;

    ProfilerGPU* gpuProfiler = nullptr;
    ProfilerCPU* cpuProfiler = nullptr;
    const char* cpuCounterName = nullptr;

    uint32 traceHistoryOffset = 0;

    bool overlayEnabled = false;
};
}