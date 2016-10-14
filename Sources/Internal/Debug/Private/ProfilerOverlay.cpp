#include "Debug/ProfilerOverlay.h"
#include "Debug/DVAssert.h"
#include "Debug/DebugColors.h"
#include "Debug/TraceEvent.h"
#include "Debug/ProfilerGPU.h"
#include "Debug/ProfilerCPU.h"
#include "Debug/ProfilerMarkerNames.h"
#include "Render/Renderer.h"
#include "Render/RHI/dbg_Draw.h"
#include <ostream>

namespace DAVA
{
//==============================================================================

namespace ProfilerOverlayDetails
{
static const char* OVERLAY_MARKER_CPU_TIME = "OverlayCPUTime";
static const char* OVERLAY_PASS_MARKER_NAME = "OverlayRenderPass";

static const int32 MARKER_HISTORY_CHART_HEIGHT = 60;
static const uint32 MARKER_HISTORY_NON_FILTERED_COUNT = 10;

static const int32 OVERLAY_RECT_MARGIN = 3;
static const int32 OVERLAY_RECT_PADDING = 4;
static const int32 TRACE_LEGEND_ICON_SIZE = DbgDraw::NormalCharH;
static const int32 TRACE_RECT_HEIGHT = DbgDraw::NormalCharH;
static const int32 TRACE_ARROW_HEIGHT = 18;
static const int32 TRACE_LEGEND_DURATION_TEXT_WIDTH_CHARS = 12;
static const int32 MIN_HIGHLIGHTED_TRACE_RECT_SIZE = 10;

static const int32 HISTORY_CHART_TEXT_COLUMN_CHARS = 9;
static const int32 HISTORY_CHART_TEXT_COLUMN_WIDTH = DbgDraw::NormalCharW * HISTORY_CHART_TEXT_COLUMN_CHARS;
static const uint64 HISTORY_CHART_CEIL_STEP = 500; //mcs

static const uint32 MAX_CPU_FRAME_TRACES = 6;
static const uint32 MAX_TRACE_LEGENT_ELEMENTS = 9;
};

static ProfilerOverlay GLOBAL_PROFILER_OVERLAY(ProfilerCPU::globalProfiler, ProfilerCPUMarkerName::CORE_PROCESS_FRAME, ProfilerGPU::globalProfiler,
                                               {
                                               FastName(ProfilerCPUMarkerName::CORE_PROCESS_FRAME),
                                               FastName(ProfilerCPUMarkerName::CORE_UI_SYSTEM_UPDATE),
                                               FastName(ProfilerCPUMarkerName::CORE_UI_SYSTEM_DRAW),
                                               FastName(ProfilerCPUMarkerName::SCENE_UPDATE),
                                               FastName(ProfilerCPUMarkerName::SCENE_DRAW),
                                               FastName(ProfilerCPUMarkerName::RENDER_PASS_DRAW_LAYERS),
                                               FastName(ProfilerCPUMarkerName::RHI_PRESENT),
                                               FastName(ProfilerCPUMarkerName::RHI_WAIT_FRAME),
                                               FastName(ProfilerCPUMarkerName::RHI_WAIT_FRAME_EXECUTION),
                                               FastName(ProfilerCPUMarkerName::RHI_PROCESS_SCHEDULED_DELETE),
                                               FastName(ProfilerOverlayDetails::OVERLAY_MARKER_CPU_TIME),

                                               FastName(ProfilerGPUMarkerName::GPU_FRAME),
                                               FastName(ProfilerGPUMarkerName::RENDER_PASS_MAIN_3D),
                                               FastName(ProfilerGPUMarkerName::RENDER_PASS_WATER_REFLECTION),
                                               FastName(ProfilerGPUMarkerName::RENDER_PASS_WATER_REFRACTION),
                                               FastName(ProfilerGPUMarkerName::RENDER_PASS_2D),
                                               FastName(ProfilerGPUMarkerName::LANDSCAPE),
                                               FastName(ProfilerOverlayDetails::OVERLAY_PASS_MARKER_NAME)
                                               });

ProfilerOverlay* const ProfilerOverlay::globalProfilerOverlay = &GLOBAL_PROFILER_OVERLAY;

ProfilerOverlay::ProfilerOverlay(ProfilerCPU* _cpuProfiler, const char* _cpuCounterName, ProfilerGPU* _gpuProfiler, const Vector<FastName>& _interestMarkers)
    : interestMarkers(_interestMarkers)
    , gpuProfiler(_gpuProfiler)
    , cpuProfiler(_cpuProfiler)
    , cpuCounterName(_cpuCounterName)
{
    for (RingArray<TraceData>& t : tracesData)
        t = RingArray<TraceData>(std::size_t(TRACE_HISTORY_SIZE));
}

void ProfilerOverlay::Enable()
{
    overlayEnabled = true;
}

void ProfilerOverlay::Disable()
{
    overlayEnabled = false;
}

bool ProfilerOverlay::IsEnabled()
{
    return overlayEnabled;
}

void ProfilerOverlay::OnFrameEnd()
{
    if (!overlayEnabled)
        return;

    DAVA_PROFILER_CPU_SCOPE(ProfilerOverlayDetails::OVERLAY_MARKER_CPU_TIME);

    Update();
    Draw();
}

void ProfilerOverlay::SetDrawScace(float32 scale)
{
    drawScale = scale;
}

float32 ProfilerOverlay::GetDrawScale() const
{
    return drawScale;
}

void ProfilerOverlay::SetCPUProfiler(ProfilerCPU* profiler, const char* counterName)
{
    cpuProfiler = profiler;
    cpuCounterName = counterName;

    if (selectedTrace == TRACE_CPU && !cpuProfiler)
        selectedTrace = TRACE_GPU;
}

void ProfilerOverlay::SetGPUProfiler(ProfilerGPU* profiler)
{
    gpuProfiler = profiler;
    if (selectedTrace == TRACE_GPU && !gpuProfiler)
        selectedTrace = TRACE_GPU;
}

void ProfilerOverlay::ClearInterestMarkers()
{
    interestMarkers.clear();
}

void ProfilerOverlay::AddInterestMarker(const FastName& name)
{
    interestMarkers.push_back(name);
}

Vector<FastName> ProfilerOverlay::GetAvalibleMarkers() const
{
    Vector<FastName> ret;
    for (const TraceData::LegentElement& e : tracesData[TRACE_CPU].crbegin()->legend)
        ret.push_back(e.name);

    for (const TraceData::LegentElement& e : tracesData[TRACE_GPU].crbegin()->legend)
        ret.push_back(e.name);

    return ret;
}

void ProfilerOverlay::SelectNextMarker()
{
    const TraceData& data = GetHistoricTrace(tracesData[selectedTrace]);
    int32 selectedIndex = FindLegendIndex(data.legend, selectedMarkers[selectedTrace]);
    if (selectedIndex == -1)
    {
        selectedIndex = 0;
    }
    else if (selectedIndex < int32(data.legend.size() - 1))
    {
        ++selectedIndex;
    }

    if (selectedIndex < int32(data.legend.size()))
        selectedMarkers[selectedTrace] = data.legend[selectedIndex].name;
}

void ProfilerOverlay::SelectPreviousMarker()
{
    const TraceData& data = GetHistoricTrace(tracesData[selectedTrace]);
    int32 selectedIndex = FindLegendIndex(data.legend, selectedMarkers[selectedTrace]);
    if (selectedIndex == -1)
    {
        selectedIndex = 0;
    }
    else if (selectedIndex > 0)
    {
        --selectedIndex;
    }

    if (selectedIndex < int32(data.legend.size()))
        selectedMarkers[selectedTrace] = data.legend[selectedIndex].name;
}

void ProfilerOverlay::SelectMarker(const FastName& name)
{
    const TraceData& data = GetHistoricTrace(tracesData[selectedTrace]);
    if (FindLegendIndex(data.legend, name) != -1)
        selectedMarkers[selectedTrace] = name;
}

void ProfilerOverlay::SelectTrace(eTrace trace)
{
    selectedTrace = trace;
}

ProfilerOverlay::eTrace ProfilerOverlay::GetSelectedTrace()
{
    return selectedTrace;
}

void ProfilerOverlay::SetTraceHistoryOffset(uint32 offset)
{
    traceHistoryOffset = Clamp(offset, 0U, TRACE_HISTORY_SIZE - 1);
}

uint32 ProfilerOverlay::GetTraceHistoryOffset() const
{
    return traceHistoryOffset;
}

void ProfilerOverlay::Update()
{
    bool needUpdateCPUInfo = false;
    bool needUpdateGPUInfo = false;

    if (gpuProfiler)
    {
        uint32 lastGPUFrameIndex = gpuProfiler->GetLastFrame().frameIndex;
        if (tracesData[TRACE_GPU].crbegin()->frameIndex != lastGPUFrameIndex)
        {
            while (!CPUFrameTraces.empty() && (CPUFrameTraces.front().frameIndex < lastGPUFrameIndex))
                CPUFrameTraces.pop_front();

            needUpdateCPUInfo = !CPUFrameTraces.empty();
            needUpdateGPUInfo = true;
        }

        while (uint32(CPUFrameTraces.size()) > ProfilerOverlayDetails::MAX_CPU_FRAME_TRACES)
            CPUFrameTraces.pop_front();
    }
    else if (cpuProfiler)
    {
        needUpdateCPUInfo = !CPUFrameTraces.empty();
    }

    bool needUpdateHistory = needUpdateCPUInfo || needUpdateGPUInfo;
    if (needUpdateHistory)
    {
        for (FastNameMap<MarkerHistory>::HashMapItem& i : markersHistory)
        {
            MarkerHistory::HistoryArray& history = i.second.values;
            MarkerHistory::HistoryInstance& value = history.next();
            value.accurate = 0;
            value.filtered = 0.f;
        }
    }

    if (needUpdateGPUInfo)
    {
        ProcessEventsTrace(gpuProfiler->GetLastFrame().GetTrace(), gpuProfiler->GetLastFrame().frameIndex, &tracesData[TRACE_GPU].next());
    }

    if (needUpdateCPUInfo)
    {
        ProcessEventsTrace(CPUFrameTraces.front().trace, CPUFrameTraces.front().frameIndex, &tracesData[TRACE_CPU].next());
        CPUFrameTraces.pop_front();
    }

    if (needUpdateHistory)
    {
        for (FastNameMap<MarkerHistory>::HashMapItem& i : markersHistory)
        {
            MarkerHistory& history = i.second;
            ++history.updatesCount;

            MarkerHistory::HistoryInstance& current = *history.values.rbegin();
            if (history.updatesCount < ProfilerOverlayDetails::MARKER_HISTORY_NON_FILTERED_COUNT)
            {
                current.filtered = float32(current.accurate);
            }
            else
            {
                const MarkerHistory::HistoryInstance& prev = *(history.values.crbegin() + 1);
                current.filtered = prev.filtered * 0.99f + current.accurate * 0.01f;
            }
        }
    }

    if (cpuProfiler && cpuProfiler->IsStarted())
    {
        uint32 currentFrameIndex = Core::Instance()->GetGlobalFrameIndex();
        CPUFrameTraces.push_back({ cpuProfiler->GetTrace(cpuCounterName), (currentFrameIndex - 1) });
    }
}

void ProfilerOverlay::ProcessEventsTrace(const Vector<TraceEvent>& events, uint32 frameIndex, TraceData* trace)
{
    trace->frameIndex = frameIndex;
    trace->minTimestamp = uint64(-1);
    trace->maxTimestamp = 0;
    trace->rects.clear();

    uint64 eventStart, eventDuration;
    uint32 eventColor;
    int32 eventDepth;

    Vector<std::pair<uint64, uint64>> timestampsStack; //<start ts, end ts>
    for (const TraceEvent& e : events)
    {
        MarkerHistory& history = markersHistory[e.name];
        uint64& historyMarkerDuration = history.values.rbegin()->accurate;

        switch (e.phase)
        {
        case TraceEvent::PHASE_DURATION:
            historyMarkerDuration += e.duration;
            trace->maxTimestamp = Max(trace->maxTimestamp, e.timestamp + e.duration);
            break;

        case TraceEvent::PHASE_BEGIN:
            historyMarkerDuration -= e.timestamp;
            break;

        case TraceEvent::PHASE_END:
            historyMarkerDuration += e.timestamp;
            trace->maxTimestamp = Max(trace->maxTimestamp, e.timestamp);
            break;

        default:
            break;
        }

        trace->minTimestamp = Min(trace->minTimestamp, e.timestamp);

        auto found = std::find_if(trace->legend.begin(), trace->legend.end(), [&e](const TraceData::LegentElement& element) {
            return (element.name == e.name);
        });

        if (found == trace->legend.end())
        {
            trace->legend.push_back({ e.name, 0 });
            trace->maxMarkerNameLen = Max(trace->maxMarkerNameLen, uint32(strlen(e.name.c_str())));
        }

        if (markersColor.count(e.name) == 0)
        {
            static uint32 colorIndex = 0;
            markersColor.Insert(e.name, rhi::NativeColorRGBA(CIEDE2000Colors[colorIndex % CIEDE2000_COLORS_COUNT]));
            ++colorIndex;
        }

        //////////////////////////////////////////////////////////////////////////

        while (!timestampsStack.empty() && (timestampsStack.back().second != 0) && (e.timestamp >= timestampsStack.back().second))
            timestampsStack.pop_back();

        switch (e.phase)
        {
        case TraceEvent::PHASE_DURATION:
            timestampsStack.emplace_back(std::pair<uint64, uint64>(e.timestamp, e.timestamp + e.duration));
            break;

        case TraceEvent::PHASE_BEGIN:
            timestampsStack.emplace_back(std::pair<uint64, uint64>(e.timestamp, 0));
            break;

        case TraceEvent::PHASE_END:
            timestampsStack.back().second = e.timestamp;
            break;

        default:
            break;
        }

        if (e.phase == TraceEvent::PHASE_END || e.phase == TraceEvent::PHASE_DURATION)
        {
            DVASSERT(!timestampsStack.empty());

            eventStart = timestampsStack.back().first - events.front().timestamp;
            eventDuration = timestampsStack.back().second - timestampsStack.back().first;
            eventColor = markersColor[e.name];
            eventDepth = int32(timestampsStack.size()) - 1;

            trace->rects.push_back({ eventStart, eventDuration, eventColor, eventDepth, e.name });
        }
    }

    for (TraceData::LegentElement& e : trace->legend)
        e.duration = markersHistory[e.name].values.rbegin()->accurate;
}

void ProfilerOverlay::Draw()
{
    Size2i screenSize(int32(Renderer::GetFramebufferWidth() / drawScale), int32(Renderer::GetFramebufferHeight() / drawScale));

    DbgDraw::EnsureInited();
    DbgDraw::SetScreenSize(screenSize.dx, screenSize.dy);
    DbgDraw::SetNormalTextSize();

    TraceData& currentCPUTrace = GetHistoricTrace(tracesData[TRACE_CPU]);
    TraceData& currentGPUTrace = GetHistoricTrace(tracesData[TRACE_GPU]);

    Rect2i rect = Rect2i(0, 0, screenSize.dx, GetEnoughRectHeight(currentCPUTrace));
    DrawTrace(currentCPUTrace, Format("CPU Frame %d", currentCPUTrace.frameIndex).c_str(), rect, selectedMarkers[TRACE_CPU], (selectedTrace == TRACE_CPU));

    rect.y += rect.dy;
    rect.dy = GetEnoughRectHeight(currentGPUTrace);
    DrawTrace(currentGPUTrace, Format("GPU Frame %d", currentGPUTrace.frameIndex).c_str(), rect, selectedMarkers[TRACE_GPU], (selectedTrace == TRACE_GPU));

    int32 chartColumnCount = (screenSize.dy - rect.y - rect.dy) / ProfilerOverlayDetails::MARKER_HISTORY_CHART_HEIGHT;
    int32 chartRowCount = int32(ceilf(float32(interestMarkers.size()) / chartColumnCount));
    int32 chartWidth = screenSize.dx / chartRowCount;
    int32 chartTableY = rect.y + rect.dy;

    rect.x = 0;
    rect.y = chartTableY;
    rect.dy = ProfilerOverlayDetails::MARKER_HISTORY_CHART_HEIGHT;
    rect.dx = chartWidth;
    for (const FastName& m : interestMarkers)
    {
        DrawHistory(m, rect);
        rect.y += rect.dy;
        if ((rect.y + rect.dy) > screenSize.dy)
        {
            rect.x += chartWidth;
            rect.y = chartTableY;
        }
    }

    //////////////////////////////////////////////////////////////////////////

    rhi::RenderPassConfig passConfig;
    passConfig.colorBuffer[0].loadAction = rhi::LOADACTION_LOAD;
    passConfig.colorBuffer[0].storeAction = rhi::STOREACTION_STORE;
    passConfig.depthStencilBuffer.loadAction = rhi::LOADACTION_NONE;
    passConfig.depthStencilBuffer.storeAction = rhi::STOREACTION_NONE;
    passConfig.priority = PRIORITY_MAIN_2D - 10;
    passConfig.viewport.x = 0;
    passConfig.viewport.y = 0;
    passConfig.viewport.width = Renderer::GetFramebufferWidth();
    passConfig.viewport.height = Renderer::GetFramebufferHeight();
    DAVA_PROFILER_GPU_RENDER_PASS(passConfig, ProfilerOverlayDetails::OVERLAY_PASS_MARKER_NAME);

    rhi::HPacketList packetList;
    rhi::HRenderPass pass = rhi::AllocateRenderPass(passConfig, 1, &packetList);
    rhi::BeginRenderPass(pass);
    rhi::BeginPacketList(packetList);

    DbgDraw::FlushBatched(packetList);

    rhi::EndPacketList(packetList);
    rhi::EndRenderPass(pass);
}

void ProfilerOverlay::DrawTrace(const TraceData& trace, const char* traceHeader, const Rect2i& rect, const FastName& selectedMarker, bool traceSelected)
{
    static const uint32 BACKGROUND_COLOR = rhi::NativeColorRGBA(0.f, 0.f, .6f, .4f);
    static const uint32 SELECTED_BACKGROUND_COLOR = rhi::NativeColorRGBA(0.f, 0.f, 1.f, .4f);
    static const uint32 TEXT_COLOR = rhi::NativeColorRGBA(1.f, 1.f, 1.f, 1.f);
    static const uint32 SELECTED_COLOR = rhi::NativeColorRGBA(1.f, 0.f, 0.f, 1.f);
    static const uint32 ARROW_COLOR = rhi::NativeColorRGBA(1.f, 0.f, 0.f, 1.f);
    static const uint32 ARROW_OUTLINE_COLOR = rhi::NativeColorRGBA(.4f, 0.f, 0.f, 1.f);
    static const uint32 LINE_COLOR = rhi::NativeColorRGBA(.5f, 0.f, 0.f, 1.f);

    static const int32 MARGIN = ProfilerOverlayDetails::OVERLAY_RECT_MARGIN;
    static const int32 PADDING = ProfilerOverlayDetails::OVERLAY_RECT_PADDING;

    Rect2i drawRect(rect);
    drawRect.x += PADDING / 2;
    drawRect.y += PADDING / 2;
    drawRect.dx -= PADDING;
    drawRect.dy -= PADDING;

    DbgDraw::FilledRect2D(drawRect.x, drawRect.y, drawRect.x + drawRect.dx, drawRect.y + drawRect.dy, traceSelected ? SELECTED_BACKGROUND_COLOR : BACKGROUND_COLOR);

    int32 x0, x1, y0, y1;

    //Draw Head
    x0 = drawRect.x + MARGIN;
    y0 = drawRect.y + MARGIN;
    DbgDraw::Text2D(x0, y0, TEXT_COLOR, traceHeader);

    //Draw Legend (color rects + marker name) and total markers duration
    int32 legentWidth = ProfilerOverlayDetails::TRACE_LEGEND_ICON_SIZE + DbgDraw::NormalCharW + trace.maxMarkerNameLen * DbgDraw::NormalCharW + DbgDraw::NormalCharW;
    y0 += DbgDraw::NormalCharH + MARGIN;
    x1 = x0 + ProfilerOverlayDetails::TRACE_LEGEND_ICON_SIZE;

    char strbuf[256];
    uint32 textColor = 0;

    int32 selectedIndex = FindLegendIndex(trace.legend, selectedMarker);
    int32 maxElements = int32(ProfilerOverlayDetails::MAX_TRACE_LEGENT_ELEMENTS);
    int32 elementsCount = int32(trace.legend.size());

    int32 startIndex = Min(Max(selectedIndex - maxElements / 2, 0), Max(elementsCount - maxElements, 0));
    int32 endIndex = Min(elementsCount, startIndex + maxElements);
    for (int32 i = startIndex; i < endIndex; ++i)
    {
        const TraceData::LegentElement& element = trace.legend[i];
        textColor = (element.name == selectedMarker) ? SELECTED_COLOR : TEXT_COLOR;

        y1 = y0 + ProfilerOverlayDetails::TRACE_LEGEND_ICON_SIZE;

        DbgDraw::FilledRect2D(x0, y0, x1, y1, markersColor[element.name]);
        DbgDraw::Text2D(x1 + DbgDraw::NormalCharW, y0, textColor, element.name.c_str());

        snprintf(strbuf, countof(strbuf), "[%*d mcs]", ProfilerOverlayDetails::TRACE_LEGEND_DURATION_TEXT_WIDTH_CHARS - 6, uint32(element.duration));
        DbgDraw::Text2D(x0 + legentWidth, y0, textColor, strbuf);

        y0 += ProfilerOverlayDetails::TRACE_LEGEND_ICON_SIZE + 1;
    }

    //Draw selected marker history
    if (selectedMarker.IsValid())
        DrawHistory(selectedMarker, Rect2i(rect.x + PADDING / 2, y0, rect.dx - PADDING, ProfilerOverlayDetails::MARKER_HISTORY_CHART_HEIGHT), false);

    //Draw separators
    x0 = drawRect.x + MARGIN;
    x1 = drawRect.x + drawRect.dx - MARGIN;
    ;
    DbgDraw::Line2D(x0, y0, x1, y0, LINE_COLOR);

    int32 durationTextWidth = ProfilerOverlayDetails::TRACE_LEGEND_DURATION_TEXT_WIDTH_CHARS * DbgDraw::NormalCharW;
    x0 = drawRect.x + MARGIN + legentWidth + durationTextWidth + MARGIN;
    x1 = x0;
    y1 = y0;
    y0 = drawRect.y + MARGIN + DbgDraw::NormalCharH + MARGIN;
    DbgDraw::Line2D(x0, y0, x1, y1, LINE_COLOR);

    //Draw traces rects
    int32 x0trace = x0 + MARGIN;
    int32 y0trace = y0;
    int32 tracedx = drawRect.dx - x0trace - MARGIN;
    float32 dt = float32(tracedx) / (trace.maxTimestamp - trace.minTimestamp);

    Vector<const TraceData::TraceRect*> arrowedRects;
    for (const TraceData::TraceRect& r : trace.rects)
    {
        x0 = x0trace + int32(r.start * dt);
        x1 = x0 + Max(1, int32(r.duration * dt));
        y0 = y0trace + int32(r.depth * ProfilerOverlayDetails::TRACE_RECT_HEIGHT);
        y1 = y0 + ProfilerOverlayDetails::TRACE_RECT_HEIGHT;

        bool drawArrow = (r.name == selectedMarker) && ((x1 - x0) < ProfilerOverlayDetails::MIN_HIGHLIGHTED_TRACE_RECT_SIZE);
        if (drawArrow)
        {
            arrowedRects.push_back(&r);
        }

        if (r.name != selectedMarker || drawArrow)
        {
            DbgDraw::FilledRect2D(x0, y0, x1, y1, r.color);
        }
        else
        {
            DbgDraw::FilledRect2D(x0, y0, x1, y1, SELECTED_COLOR);
        }
    }

    //draw arrows
    for (const TraceData::TraceRect* r : arrowedRects)
    {
        x0 = x0trace + int32(r->start * dt);
        x1 = x0 + int32(r->duration * dt);
        y0 = y0trace + int32(r->depth * ProfilerOverlayDetails::TRACE_RECT_HEIGHT);
        y1 = y0 + ProfilerOverlayDetails::TRACE_RECT_HEIGHT;

        int32 xm = x0 + (x1 - x0) / 2;

        DbgDraw::FilledRect2D(xm - 1, y1, xm + 2, y1 + ProfilerOverlayDetails::TRACE_ARROW_HEIGHT + 1, ARROW_OUTLINE_COLOR);
        DbgDraw::FilledRect2D(xm - 2, y1 + 2, xm + 3, y1 + 4, ARROW_OUTLINE_COLOR);
        DbgDraw::FilledRect2D(xm - 3, y1 + 4, xm + 4, y1 + 7, ARROW_OUTLINE_COLOR);

        DbgDraw::FilledRect2D(xm, y1, xm + 1, y1 + ProfilerOverlayDetails::TRACE_ARROW_HEIGHT, ARROW_COLOR);
        DbgDraw::FilledRect2D(xm - 1, y1 + 2, xm + 2, y1 + 4, ARROW_COLOR);
        DbgDraw::FilledRect2D(xm - 2, y1 + 4, xm + 3, y1 + 6, ARROW_COLOR);
    }
}

void ProfilerOverlay::DrawHistory(const FastName& name, const Rect2i& rect, bool drawBackground)
{
    static const uint32 CHARTRECT_COLOR = rhi::NativeColorRGBA(0.f, 0.f, 1.f, .4f);
    static const uint32 CHART_COLOR = rhi::NativeColorRGBA(.5f, .11f, .11f, 1.f);
    static const uint32 CHART_FILTERED_COLOR = rhi::NativeColorRGBA(1.f, .18f, .18f, 1.f);
    static const uint32 TEXT_COLOR = rhi::NativeColorRGBA(1.f, 1.f, 1.f, 1.f);
    static const uint32 LINE_COLOR = rhi::NativeColorRGBA(.5f, 0.f, 0.f, 1.f);

    static const int32 MARGIN = ProfilerOverlayDetails::OVERLAY_RECT_MARGIN;
    static const int32 PADDING = ProfilerOverlayDetails::OVERLAY_RECT_PADDING;

    const MarkerHistory::HistoryArray& history = markersHistory[name].values;

    Rect2i drawRect(rect);
    drawRect.x += PADDING / 2;
    drawRect.y += PADDING / 2;
    drawRect.dx -= PADDING;
    drawRect.dy -= PADDING;

    Rect2i chartRect(drawRect);
    chartRect.x += MARGIN + ProfilerOverlayDetails::HISTORY_CHART_TEXT_COLUMN_WIDTH + MARGIN;
    chartRect.y += MARGIN + DbgDraw::NormalCharH;
    chartRect.dx -= ProfilerOverlayDetails::HISTORY_CHART_TEXT_COLUMN_WIDTH + 3 * MARGIN;
    chartRect.dy -= 2 * MARGIN + DbgDraw::NormalCharH;

    uint64 maxValue = 0;
    for (const MarkerHistory::HistoryInstance& h : history)
        maxValue = Max(maxValue, Max(h.accurate, uint64(h.filtered)));

    char strbuf[128];
    float32 ceilValue = float32((maxValue / ProfilerOverlayDetails::HISTORY_CHART_CEIL_STEP + 1) * ProfilerOverlayDetails::HISTORY_CHART_CEIL_STEP);

    if (drawBackground)
        DbgDraw::FilledRect2D(drawRect.x, drawRect.y, drawRect.x + drawRect.dx, drawRect.y + drawRect.dy, CHARTRECT_COLOR);

    DbgDraw::Line2D(chartRect.x, chartRect.y, chartRect.x, chartRect.y + chartRect.dy, LINE_COLOR);
    DbgDraw::Line2D(chartRect.x, chartRect.y + chartRect.dy, chartRect.x + chartRect.dx, chartRect.y + chartRect.dy, LINE_COLOR);

    const float32 chartstep = float32(chartRect.dx) / history.size();
    const float32 valuescale = chartRect.dy / ceilValue;

    const int32 chart0x = chartRect.x;
    const int32 chart0y = chartRect.y + chartRect.dy;

#define CHART_VALUE_HEIGHT(value) int32(value* valuescale)

    MarkerHistory::HistoryArray::const_iterator it = history.cbegin();
    int32 px = 0;
    int32 py = CHART_VALUE_HEIGHT(it->accurate);
    int32 pfy = CHART_VALUE_HEIGHT(it->filtered);
    ++it;

    int32 index = 1;
    MarkerHistory::HistoryArray::const_iterator hend = history.cend();
    for (; it != hend; ++it, ++index)
    {
        int32 x = int32(index * chartstep);
        int32 y = CHART_VALUE_HEIGHT(it->accurate);
        int32 fy = CHART_VALUE_HEIGHT(it->filtered);

        DbgDraw::Line2D(chart0x + px, chart0y - py, chart0x + x, chart0y - y, CHART_COLOR);
        DbgDraw::Line2D(chart0x + px, chart0y - pfy, chart0x + x, chart0y - fy, CHART_FILTERED_COLOR);

        px = x;
        py = y;
        pfy = fy;
    }

#undef CHART_VALUE_HEIGHT

    DbgDraw::Text2D(drawRect.x + MARGIN, drawRect.y + MARGIN, TEXT_COLOR, "\'%s\'", name.c_str());

    snprintf(strbuf, countof(strbuf), "%lld [%.1f] mcs", history.crbegin()->accurate, history.crbegin()->filtered);
    int32 tdx = drawRect.dx - MARGIN - int32(DbgDraw::NormalCharW * strlen(strbuf));
    DbgDraw::Text2D(drawRect.x + tdx, drawRect.y + MARGIN, TEXT_COLOR, strbuf);

    snprintf(strbuf, countof(strbuf), "%d mcs", int32(ceilValue));
    DbgDraw::Text2D(drawRect.x + MARGIN, drawRect.y + MARGIN + DbgDraw::NormalCharH, TEXT_COLOR, "%*s", ProfilerOverlayDetails::HISTORY_CHART_TEXT_COLUMN_CHARS, strbuf);
    DbgDraw::Text2D(drawRect.x + MARGIN, drawRect.y + drawRect.dy - MARGIN - DbgDraw::NormalCharH, TEXT_COLOR, "%*s", ProfilerOverlayDetails::HISTORY_CHART_TEXT_COLUMN_CHARS, "0 mcs");
}

int32 ProfilerOverlay::GetEnoughRectHeight(const TraceData& trace)
{
    uint32 legendCount = Min(uint32(trace.legend.size()), ProfilerOverlayDetails::MAX_TRACE_LEGENT_ELEMENTS);
    const int32 MARGIN = ProfilerOverlayDetails::OVERLAY_RECT_MARGIN;
    return MARGIN + DbgDraw::NormalCharH + MARGIN + int32(legendCount) * (DbgDraw::NormalCharH + 1) + ProfilerOverlayDetails::MARKER_HISTORY_CHART_HEIGHT + ProfilerOverlayDetails::OVERLAY_RECT_PADDING;
}

int32 ProfilerOverlay::FindLegendIndex(const Vector<TraceData::LegentElement>& legend, const FastName& marker)
{
    auto found = std::find_if(legend.begin(), legend.end(), [&marker](const TraceData::LegentElement& element) {
        return (element.name == marker);
    });

    if (found != legend.end())
        return int32(std::distance(legend.begin(), found));

    return -1;
}

ProfilerOverlay::TraceData& ProfilerOverlay::GetHistoricTrace(RingArray<TraceData>& traceData)
{
    return *(traceData.rbegin() + traceHistoryOffset);
}

}; //ns
