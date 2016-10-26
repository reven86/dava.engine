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
    static const std::size_t TRACE_HISTORY_SIZE = 10;
    static ProfilerOverlay* const globalProfilerOverlay;

    enum eTrace
    {
        TRACE_CPU = 0,
        TRACE_GPU,

        TRACE_COUNT
    };

    ProfilerOverlay(ProfilerCPU* cpuProfiler, const char* cpuCounterName, ProfilerGPU* gpuProfiler, const Vector<FastName>& interestMarkers = Vector<FastName>());

    void SetEnabled(bool enabled);
    bool IsEnabled();

    void SetInputEnabled(bool enabled);
    bool IsInputEnabled();

    void SetPaused(bool paused);
    bool IsPaused() const;

    void SetDrawScace(float32 scale);
    float32 GetDrawScale() const;

    void OnFrameEnd(); //should be called before rhi::Present();
    bool OnInput(UIEvent* input); //return 'true' if input processed and 'false' otherwise

    void SetCPUProfiler(ProfilerCPU* profiler, const char* counterName);

    //marker's history control
    void ClearInterestMarkers();
    void AddInterestMarker(const FastName& name);
    void AddInterestMarkers(const Vector<FastName>& markers);

    const Vector<FastName> GetInterestMarkers() const;
    Vector<FastName> GetAvalibleMarkers() const;

    //selection control
    void SelectNextMarker();
    void SelectPreviousMarker();
    void SelectMarker(const FastName& name);

    void SelectTrace(eTrace trace);
    eTrace GetSelectedTrace();

    //trace history control
    void SetTraceHistoryOffset(uint32 offset);
    uint32 GetTraceHistoryOffset() const;

protected:
    static const uint32 MARKER_HISTORY_LENGTH = 120;

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

        struct ListElement
        {
            FastName name;
            uint64 duration;
        };

        Vector<TraceRect> rects;
        Vector<ListElement> list;
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

    enum eButton
    {
        BUTTON_CPU_UP = 0,
        BUTTON_CPU_DOWN,
        BUTTON_GPU_UP,
        BUTTON_GPU_DOWN,
        BUTTON_HISTORY_PREV,
        BUTTON_HISTORY_NEXT,
        BUTTON_DRAW_MARKER_HISTORY,
        BUTTON_PROFILERS_START_STOP,
        BUTTON_CLOSE,

        BUTTON_COUNT
    };

    void Update();
    void ProcessEventsTrace(const Vector<TraceEvent>& events, TraceData* trace);

    void Draw();
    void DrawTrace(const TraceData& trace, const char* traceHeader, const Rect2i& rect, const FastName& selectedMarker, bool traceSelected, Rect2i* upButton, Rect2i* downButton);
    void DrawHistory(const FastName& name, const Rect2i& rect, bool drawBackground = true);

    int32 GetEnoughRectHeight(const TraceData& trace);
    int32 FindListIndex(const Vector<TraceData::ListElement>& legend, const FastName& marker);
    TraceData& GetHistoricTrace(RingArray<TraceData>& traceData);

    void Reset();

    void ProcessTouch(UIEvent* input);
    void OnButtonPressed(eButton button);

    FastNameMap<MarkerHistory> markersHistory = FastNameMap<MarkerHistory>(128, MarkerHistory({ MarkerHistory::HistoryArray(MARKER_HISTORY_LENGTH), 0 }));
    FastNameMap<uint32> markersColor;
    Vector<FastName> interestMarkers;

    RingArray<TraceData> tracesData[TRACE_COUNT];
    eTrace selectedTrace = TRACE_CPU;

    FastName selectedMarkers[TRACE_COUNT];

    ProfilerGPU* gpuProfiler = nullptr;
    ProfilerCPU* cpuProfiler = nullptr;
    const char* cpuCounterName = nullptr;

    uint32 traceHistoryOffset = 0;
    uint32 colorIndex = 0;
    float32 overlayScale = 1.f;

    Rect2i buttons[BUTTON_COUNT];
    const char* buttonsText[BUTTON_COUNT];

    bool overlayEnabled = false;
    bool overlayPaused = true;
    bool inputEnabled = false;
    bool drawMarkerHistory = false;
};

inline void ProfilerOverlay::SetEnabled(bool enabled)
{
    overlayEnabled = enabled;
}

inline bool ProfilerOverlay::IsEnabled()
{
    return overlayEnabled;
}

inline void ProfilerOverlay::SetInputEnabled(bool enabled)
{
    inputEnabled = enabled;
}

inline bool ProfilerOverlay::IsInputEnabled()
{
    return inputEnabled;
}

inline void ProfilerOverlay::SetDrawScace(float32 scale)
{
    overlayScale = scale;
}

inline float32 ProfilerOverlay::GetDrawScale() const
{
    return overlayScale;
}

inline void ProfilerOverlay::SelectTrace(eTrace trace)
{
    selectedTrace = trace;
}

inline ProfilerOverlay::eTrace ProfilerOverlay::GetSelectedTrace()
{
    return selectedTrace;
}

inline void ProfilerOverlay::SetTraceHistoryOffset(uint32 offset)
{
    traceHistoryOffset = Clamp(offset, 0U, uint32(TRACE_HISTORY_SIZE) - 1);
}

inline uint32 ProfilerOverlay::GetTraceHistoryOffset() const
{
    return traceHistoryOffset;
}
}