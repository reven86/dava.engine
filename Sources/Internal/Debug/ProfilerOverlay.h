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
    ProfilerOverlay(CPUProfiler* cpuProfiler, GPUProfiler* gpuProfiler, const Vector<FastName>& interestEvents = Vector<FastName>());

    void Enable();
    void Disable();

    void OnFrameEnd(); //should be called before rhi::Present();

    void ClearInterestEvents();
    void AddInterestEvent(const FastName& name);
    Vector<FastName> GetAvalibleEventsNames();

    static ProfilerOverlay* const globalProfilerOverlay;

protected:
    static const uint32 EVENT_HISTORY_LENGTH = 300;
    static const uint32 FRAME_TRACE_HISTORY_LENGTH = 10;

    using HistoryInstance = std::pair<uint64, float32>; //<accurate value, filtered value>
    using HistoryArray = RingArray<HistoryInstance>;
    using EventHistory = std::pair<uint32, HistoryArray>; //<updates count, history values>

    struct OverlayTrace
    {
        struct TraceRect
        {
            TraceRect(uint64 _start, uint64 _duration, uint32 _color, int32 _depth)
                : start(_start)
                , duration(_duration)
                , color(_color)
                , depth(_depth)
            {
            }

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

    void Update();
    void UpdateCurrentTrace(OverlayTrace& trace, const Vector<TraceEvent>& events, uint32 frameIndex);

    void Draw();
    void DrawTrace(const OverlayTrace& trace, const char* traceHeader, const Rect2i& rect);
    void DrawHistory(const HistoryArray& history, const FastName& name, const Rect2i& rect);

    int32 GetEnoughRectHeight(const OverlayTrace& trace);

    FastNameMap<EventHistory> eventsHistory = FastNameMap<EventHistory>(128, EventHistory(0, HistoryArray(EVENT_HISTORY_LENGTH)));
    FastNameMap<uint32> eventsColor;

    Vector<FastName> interestEventsNames;
    uint32 maxEventNameLen = 0;

    OverlayTrace currentGPUTrace;
    OverlayTrace currentCPUTrace;
    List<std::pair<uint32, Vector<TraceEvent>>> CPUTraces; //<frameIndex, cpu-trace>

    CPUProfiler* cpuProfiler = nullptr;
    GPUProfiler* gpuProfiler = nullptr;

    bool overlayEnabled = false;
};
}