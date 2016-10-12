#pragma once

#include "Base/BaseTypes.h"
#include "Base/FastNameMap.h"
#include "Debug/Private/RingArray.h"
#include "Debug/TraceEvent.h"

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

    enum eTrace
    {
        TRACE_CPU = 0,
        TRACE_GPU,

        TRACE_COUNT
    };

    ProfilerOverlay(ProfilerCPU* cpuProfiler, const char* cpuCounterName, ProfilerGPU* gpuProfiler, const Vector<FastName>& interestMarkers = Vector<FastName>());

    void Enable();
    void Disable();
    bool IsEnabled();

    void OnFrameEnd(); //should be called before rhi::Present();

    void SetCPUProfiler(ProfilerCPU* profiler, const char* counterName);
    void SetGPUProfiler(ProfilerGPU* profiler);

    //marker's history control
    void ClearInterestMarkers();
    void AddInterestMarker(const FastName& name);
    Vector<FastName> GetAvalibleMarkers() const;

    //selection control
    void SelectTrace(eTrace trace);
    eTrace GetSelectedTrace();

    void SelectNextMarker();
    void SelectPreviousMarker();
    void SelectMarker(const FastName& name);

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
    void DrawTrace(const TraceData& trace, const char* traceHeader, const Rect2i& rect, const FastName& selectedMarker, bool traceSelected = true);
    void DrawHistory(const FastName& name, const Rect2i& rect, bool drawBackground = true);

    int32 GetEnoughRectHeight(const TraceData& trace);
    int32 FindLegendIndex(const Vector<TraceData::LegentElement>& legend, const FastName& marker);
    TraceData& GetHistoricTrace(RingArray<TraceData>& traceData);

    FastNameMap<MarkerHistory> markersHistory = FastNameMap<MarkerHistory>(128, MarkerHistory({ MarkerHistory::HistoryArray(MARKER_HISTORY_LENGTH), 0 }));
    FastNameMap<uint32> markersColor;

    Vector<FastName> interestMarkers;
    List<FrameTrace> CPUFrameTraces;

    RingArray<TraceData> tracesData[TRACE_COUNT];
    eTrace selectedTrace = TRACE_CPU;

    FastName selectedMarkers[TRACE_COUNT];

    ProfilerGPU* gpuProfiler = nullptr;
    ProfilerCPU* cpuProfiler = nullptr;
    const char* cpuCounterName = nullptr;

    uint32 traceHistoryOffset = 0;

    bool overlayEnabled = false;
};
}