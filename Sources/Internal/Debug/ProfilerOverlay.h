#pragma once

#include "Base/BaseTypes.h"
#include "Base/FastNameMap.h"
#include "Debug/Private/RingArray.h"

namespace DAVA
{
class UIEvent;
class ProfilerCPU;
class ProfilerGPU;
class ProfilerOverlay
{
public:
    static const uint32 TRACE_HISTORY_SIZE = 10;
    static ProfilerOverlay* const globalProfilerOverlay;

    ProfilerOverlay(ProfilerCPU* cpuProfiler, const char* cpuCounterName, ProfilerGPU* gpuProfiler, const Vector<FastName>& interestMarkers = Vector<FastName>());

    void Enable();
    void Disable();

    void OnFrameEnd(); //should be called before rhi::Present();

    void SetCPUProfiler(ProfilerCPU* profiler, const char* counterName);
    void SetGPUProfiler(ProfilerGPU* profiler);

    //marker's history control
    void ClearInterestMarkers();
    void AddInterestMarker(const FastName& name);
    Vector<FastName> GetAvalibleMarkers() const;

    //selection control
    void SelectNextMarker();
    void SelectPreviousMarker();
    void SelectMarker(const FastName& name);
    void SwitchFocus();

    //trace history control
    void SetTraceHistoryOffset(uint32 offset);
    uint32 GetTraceHistoryOffset() const;

protected:
    static const uint32 MARKER_HISTORY_LENGTH = 300;

    struct MarkerHistory
    {
        struct HistoryInstance
        {
            uint64 accurate = 0;
            float32 filtered = 0.f;
        };
        using HistoryArray = RingArray<HistoryInstance>;

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
            FastName name;
        };

        struct LegentElement
        {
            FastName name;
            uint64 duration;
        };

        Vector<TraceRect> rects;
        Vector<LegentElement> legend;
        uint32 frameIndex = 0;
        uint64 minTimestamp = uint64(-1);
        uint64 maxTimestamp = 0;
        uint32 maxMarkerNameLen = 0;
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
    void DrawHistory(const MarkerHistory::HistoryArray& history, const FastName& name, const Rect2i& rect, bool highlightTitle = false);

    int32 GetEnoughRectHeight(const TraceData& trace);
    int32 FindLegendIndex(const Vector<TraceData::LegentElement>& legend, const FastName& marker);
    TraceData& GetCurrentTrace(RingArray<TraceData>& traceData);
    TraceData& GetFocusedTrace();

    FastNameMap<MarkerHistory> markersHistory = FastNameMap<MarkerHistory>(128, MarkerHistory({ MarkerHistory::HistoryArray(MARKER_HISTORY_LENGTH), 0 }));
    FastNameMap<uint32> markersColor;

    Vector<FastName> interestMarkers;

    RingArray<TraceData> GPUTraceData = RingArray<TraceData>(std::size_t(TRACE_HISTORY_SIZE));
    RingArray<TraceData> CPUTraceData = RingArray<TraceData>(std::size_t(TRACE_HISTORY_SIZE));
    List<FrameTrace> CPUFrameTraces;
    FastName selectedMarker;

    ProfilerGPU* gpuProfiler = nullptr;
    ProfilerCPU* cpuProfiler = nullptr;
    const char* cpuCounterName = nullptr;

    uint32 traceHistoryOffset = 0;

    bool focusOnCPUTrace = true;
    bool overlayEnabled = false;
};
}