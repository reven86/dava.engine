#pragma once

#include "Base/BaseTypes.h"
#include "Base/FastNameMap.h"
#include "Base/RingArray.h"

namespace DAVA
{
class CPUProfiler;
class GPUProfiler;
class ProfilerOverlay
{
public:
    ProfilerOverlay(CPUProfiler* cpuProfiler, const char* cpuCounterName, GPUProfiler* gpuProfiler, const Vector<FastName>& interestEvents = Vector<FastName>());

    void Enable();
    void Disable();

    void OnFrameEnd(); //should be called before rhi::Present();

    void ClearInterestEvents();
    void AddInterestEvent(const FastName& name);
    Vector<FastName> GetAvalibleEventsNames();

    void SetCPUProfiler(CPUProfiler* profiler, const char* counterName);
    void SetGPUProfiler(GPUProfiler* profiler);

    static ProfilerOverlay* const globalProfilerOverlay;

protected:
    static const uint32 EVENT_HISTORY_LENGTH = 300;
    static const uint32 FRAME_TRACE_HISTORY_LENGTH = 10;

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
    void UpdateCurrentTrace(TraceData* trace, const Vector<TraceEvent>& events, uint32 frameIndex);

    void Draw();
    void DrawTrace(const TraceData& trace, const char* traceHeader, const Rect2i& rect);
    void DrawHistory(const HistoryArray& history, const FastName& name, const Rect2i& rect);

    int32 GetEnoughRectHeight(const TraceData& trace);

    FastNameMap<EventHistory> eventsHistory = FastNameMap<EventHistory>(128, EventHistory({ HistoryArray(EVENT_HISTORY_LENGTH), 0 }));
    FastNameMap<uint32> eventsColor;

    Vector<FastName> interestEventsNames;
    uint32 maxEventNameLen = 0;

    TraceData currentGPUTrace;
    TraceData currentCPUTrace;
    List<FrameTrace> CPUTraces;

    GPUProfiler* gpuProfiler = nullptr;
    CPUProfiler* cpuProfiler = nullptr;
    const char* cpuCounterName = nullptr;

    bool overlayEnabled = false;
};
}