#include "Debug/ProfilerOverlay.h"
#include "Debug/DVAssert.h"
#include "Debug/DebugColors.h"
#include "Debug/TraceEvent.h"
#include "Debug/GPUProfiler.h"
#include "Debug/CPUProfiler.h"
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

static const int32 EVENT_HISTORY_CHART_HEIGHT = 60;
static const uint32 EVENT_HISTORY_NON_FILTERED_COUNT = 10;

static const int32 OVERLAY_RECT_MARGIN = 3;
static const int32 OVERLAY_RECT_PADDING = 4;
static const int32 TRACE_LEGEND_ICON_SIZE = DbgDraw::NormalCharH;
static const int32 TRACE_RECT_HEIGHT = DbgDraw::NormalCharH;
static const int32 TRACE_LEGEND_DURATION_TEXT_WIDTH_CHARS = 12;

static const int32 HISTORY_CHART_TEXT_COLUMN_CHARS = 9;
static const int32 HISTORY_CHART_TEXT_COLUMN_WIDTH = DbgDraw::NormalCharW * HISTORY_CHART_TEXT_COLUMN_CHARS;
static const uint64 HISTORY_CHART_CEIL_STEP = 500; //mcs
};

static ProfilerOverlay GLOBAL_PROFILER_OVERLAY(CPUProfiler::globalProfiler, GPUProfiler::globalProfiler, {
                                                                                                         FastName(CPUMarkerName::CORE_PROCESS_FRAME),
                                                                                                         FastName(CPUMarkerName::CORE_UI_SYSTEM_UPDATE),
                                                                                                         FastName(CPUMarkerName::CORE_UI_SYSTEM_DRAW),
                                                                                                         FastName(CPUMarkerName::SCENE_UPDATE),
                                                                                                         FastName(CPUMarkerName::SCENE_DRAW),
                                                                                                         FastName(CPUMarkerName::RENDER_PASS_DRAW_LAYERS),
                                                                                                         FastName(CPUMarkerName::RHI_PRESENT),
                                                                                                         FastName(CPUMarkerName::RHI_WAIT_FRAME),
                                                                                                         FastName(CPUMarkerName::RHI_WAIT_FRAME_EXECUTION),
                                                                                                         FastName(CPUMarkerName::RHI_PROCESS_SCHEDULED_DELETE),
                                                                                                         FastName(ProfilerOverlayDetails::OVERLAY_MARKER_CPU_TIME),

                                                                                                         FastName(GPUMarkerName::GPU_FRAME),
                                                                                                         FastName(GPUMarkerName::RENDER_PASS_MAIN_3D),
                                                                                                         FastName(GPUMarkerName::RENDER_PASS_WATER_REFLECTION),
                                                                                                         FastName(GPUMarkerName::RENDER_PASS_WATER_REFRACTION),
                                                                                                         FastName(GPUMarkerName::RENDER_PASS_2D),
                                                                                                         FastName(GPUMarkerName::LANDSCAPE),
                                                                                                         FastName(ProfilerOverlayDetails::OVERLAY_PASS_MARKER_NAME)
                                                                                                         });

ProfilerOverlay* const ProfilerOverlay::globalProfilerOverlay = &GLOBAL_PROFILER_OVERLAY;

ProfilerOverlay::ProfilerOverlay(CPUProfiler* _cpuProfiler, GPUProfiler* _gpuProfiler, const Vector<FastName>& _interestEvents)
    : cpuProfiler(_cpuProfiler)
    , gpuProfiler(_gpuProfiler)
    , interestEventsNames(_interestEvents)
{
    for (const FastName& n : interestEventsNames)
        maxEventNameLen = Max(maxEventNameLen, strlen(n.c_str()));
}

void ProfilerOverlay::Enable()
{
    overlayEnabled = true;
}

void ProfilerOverlay::Disable()
{
    overlayEnabled = false;
}

void ProfilerOverlay::OnFrameEnd()
{
    if (!overlayEnabled)
        return;

    DAVA_CPU_PROFILER_SCOPE(ProfilerOverlayDetails::OVERLAY_MARKER_CPU_TIME);

    Update();
    Draw();
}

void ProfilerOverlay::ClearInterestEvents()
{
    interestEventsNames.clear();
    maxEventNameLen = 0;
}

void ProfilerOverlay::AddInterestEvent(const FastName& name)
{
    interestEventsNames.push_back(name);
    maxEventNameLen = Max(maxEventNameLen, strlen(name.c_str()));
}

Vector<FastName> ProfilerOverlay::GetAvalibleEventsNames()
{
    Vector<FastName> ret;
    for (const FastName& n : currentCPUTrace.names)
        ret.push_back(n);

    for (const FastName& n : currentGPUTrace.names)
        ret.push_back(n);

    return ret;
}

void ProfilerOverlay::Update()
{
    if (!GPUProfiler::globalProfiler || !CPUProfiler::globalProfiler)
        return;

    const GPUProfiler::FrameInfo& frameInfo = GPUProfiler::globalProfiler->GetLastFrame();
    if (currentGPUTrace.frameIndex != frameInfo.frameIndex)
    {
        for (FastNameMap<EventHistory>::HashMapItem& i : eventsHistory)
        {
            HistoryArray& history = i.second.second;
            HistoryInstance& value = history.next();
            value.first = 0;
            value.second = 0.f;
        }

        UpdateCurrentTrace(currentGPUTrace, frameInfo.GetTrace(), frameInfo.frameIndex);

        while (CPUTraces.size() && CPUTraces.front().first != frameInfo.frameIndex)
            CPUTraces.pop_front();

        if (CPUTraces.size())
        {
            UpdateCurrentTrace(currentCPUTrace, CPUTraces.front().second, CPUTraces.front().first);
            CPUTraces.pop_front();
        }

        //Update history
        for (FastNameMap<EventHistory>::HashMapItem& i : eventsHistory)
        {
            EventHistory& history = i.second;
            ++history.first;

            HistoryInstance& current = *history.second.rbegin();
            if (history.first < ProfilerOverlayDetails::EVENT_HISTORY_NON_FILTERED_COUNT)
            {
                current.second = float32(current.first);
            }
            else
            {
                const HistoryInstance& prev = *(history.second.crbegin() + 1);
                current.second = prev.second * 0.99f + current.first * 0.01f;
            }
        }
    }

    uint32 currentFrameIndex = Core::Instance()->GetGlobalFrameIndex();
    CPUTraces.emplace_back((currentFrameIndex - 1), std::move(CPUProfiler::globalProfiler->GetTrace("Core::SystemProcessFrame")));
}

void ProfilerOverlay::UpdateCurrentTrace(OverlayTrace& trace, const Vector<TraceEvent>& events, uint32 frameIndex)
{
    trace.frameIndex = frameIndex;
    trace.minTimestamp = uint64(-1);
    trace.maxTimestamp = 0;
    trace.rects.clear();

    uint64 eventStart, eventDuration;
    uint32 eventColor;
    int32 eventDepth;

    Vector<std::pair<uint64, uint64>> timestampsStack; //<start ts, end ts>
    for (const TraceEvent& e : events)
    {
        EventHistory& history = eventsHistory[e.name];
        uint64& historyEventDuration = history.second.rbegin()->first;

        if (e.phase == TraceEvent::PHASE_DURATION)
        {
            historyEventDuration += e.duration;
            trace.maxTimestamp = Max(trace.maxTimestamp, e.timestamp + e.duration);
        }
        else if (e.phase == TraceEvent::PHASE_BEGIN)
        {
            historyEventDuration -= e.timestamp;
        }
        else if (e.phase == TraceEvent::PHASE_END)
        {
            historyEventDuration += e.timestamp;
            trace.maxTimestamp = Max(trace.maxTimestamp, e.timestamp);
        }

        trace.minTimestamp = Min(trace.minTimestamp, e.timestamp);

        if (eventsColor.count(e.name) == 0)
        {
            static uint32 colorIndex = 0;
            eventsColor.Insert(e.name, rhi::NativeColorRGBA(CIEDE2000Colors[colorIndex % CIEDE2000_COLORS_COUNT]));
            ++colorIndex;
        }

        trace.names.insert(e.name);

        if (eventsColor.count(e.name) == 0)
        {
            static uint32 colorIndex = 0;
            eventsColor.Insert(e.name, rhi::NativeColorRGBA(CIEDE2000Colors[colorIndex % CIEDE2000_COLORS_COUNT]));
            ++colorIndex;
        }

        //////////////////////////////////////////////////////////////////////////

        while (timestampsStack.size() && (timestampsStack.back().second != 0) && (e.timestamp >= timestampsStack.back().second))
            timestampsStack.pop_back();

        if (e.phase == TraceEvent::PHASE_DURATION)
        {
            timestampsStack.emplace_back(std::pair<uint64, uint64>(e.timestamp, e.timestamp + e.duration));
        }
        else if (e.phase == TraceEvent::PHASE_BEGIN)
        {
            timestampsStack.emplace_back(std::pair<uint64, uint64>(e.timestamp, 0));
        }
        else if (e.phase == TraceEvent::PHASE_END)
        {
            timestampsStack.back().second = e.timestamp;
        }

        if (e.phase == TraceEvent::PHASE_END || e.phase == TraceEvent::PHASE_DURATION)
        {
            DVASSERT(timestampsStack.size());

            eventStart = timestampsStack.back().first - events.front().timestamp;
            eventDuration = timestampsStack.back().second - timestampsStack.back().first;
            eventColor = eventsColor[e.name];
            eventDepth = timestampsStack.size() - 1;

            trace.rects.emplace_back(OverlayTrace::TraceRect(eventStart, eventDuration, eventColor, eventDepth));
        }
    }
}

void ProfilerOverlay::Draw()
{
    DbgDraw::EnsureInited();
    DbgDraw::SetScreenSize(uint32(Renderer::GetFramebufferWidth()), uint32(Renderer::GetFramebufferHeight()));
    DbgDraw::SetNormalTextSize();

    Rect2i rect = Rect2i(0, 0, Renderer::GetFramebufferWidth(), GetEnoughRectHeight(currentCPUTrace));
    DrawTrace(currentCPUTrace, Format("CPU Frame %d", currentCPUTrace.frameIndex).c_str(), rect);

    rect.y += rect.dy;
    rect.dy = GetEnoughRectHeight(currentGPUTrace);
    DrawTrace(currentGPUTrace, Format("GPU Frame %d", currentGPUTrace.frameIndex).c_str(), rect);

    int32 chartColumnCount = (Renderer::GetFramebufferHeight() - rect.y - rect.dy) / ProfilerOverlayDetails::EVENT_HISTORY_CHART_HEIGHT;
    int32 chartRowCount = int32(ceilf(float32(interestEventsNames.size()) / chartColumnCount));
    int32 chartWidth = Renderer::GetFramebufferWidth() / chartRowCount;
    int32 chartTableY = rect.y + rect.dy;

    rect.x = 0;
    rect.y = chartTableY;
    rect.dy = ProfilerOverlayDetails::EVENT_HISTORY_CHART_HEIGHT;
    rect.dx = chartWidth;
    for (const FastName& m : interestEventsNames)
    {
        DrawHistory(eventsHistory[m].second, m, rect);
        rect.y += rect.dy;
        if ((rect.y + rect.dy) > Renderer::GetFramebufferHeight())
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
    DAVA_GPU_PROFILER_RENDER_PASS(passConfig, ProfilerOverlayDetails::OVERLAY_PASS_MARKER_NAME);

    rhi::HPacketList packetList;
    rhi::HRenderPass pass = rhi::AllocateRenderPass(passConfig, 1, &packetList);
    rhi::BeginRenderPass(pass);
    rhi::BeginPacketList(packetList);

    DbgDraw::FlushBatched(packetList);

    rhi::EndPacketList(packetList);
    rhi::EndRenderPass(pass);
}

void ProfilerOverlay::DrawTrace(const OverlayTrace& trace, const char* traceHeader, const Rect2i& rect)
{
    static const uint32 BACKGROUND_COLOR = rhi::NativeColorRGBA(0.f, 0.f, 1.f, .4f);
    static const uint32 TEXT_COLOR = rhi::NativeColorRGBA(1.f, 1.f, 1.f, 1.f);
    static const uint32 LINE_COLOR = rhi::NativeColorRGBA(.5f, 0.f, 0.f, 1.f);

    static const int32 MARGIN = ProfilerOverlayDetails::OVERLAY_RECT_MARGIN;
    static const int32 PADDING = ProfilerOverlayDetails::OVERLAY_RECT_PADDING;

    Rect2i drawRect(rect);
    drawRect.x += PADDING / 2;
    drawRect.y += PADDING / 2;
    drawRect.dx -= 2 * PADDING / 2;
    drawRect.dy -= 2 * PADDING / 2;

    DbgDraw::FilledRect2D(drawRect.x, drawRect.y, drawRect.x + drawRect.dx, drawRect.y + drawRect.dy, BACKGROUND_COLOR);

    if (!trace.rects.size())
        return;

    int32 x0, x1, y0, y1;

    //Draw Head
    x0 = drawRect.x + MARGIN;
    y0 = drawRect.y + MARGIN;
    DbgDraw::Text2D(x0, y0, TEXT_COLOR, traceHeader);

    //Draw Legend (color rects + event name) and total events duration
    int32 legentWidth = ProfilerOverlayDetails::TRACE_LEGEND_ICON_SIZE + DbgDraw::NormalCharW + maxEventNameLen * DbgDraw::NormalCharW + DbgDraw::NormalCharW;
    y0 += DbgDraw::NormalCharH + MARGIN;
    x1 = x0 + ProfilerOverlayDetails::TRACE_LEGEND_ICON_SIZE;

    char strbuf[256];
    for (const FastName& m : interestEventsNames)
    {
        if (trace.names.count(m))
        {
            y1 = y0 + ProfilerOverlayDetails::TRACE_LEGEND_ICON_SIZE;

            DbgDraw::FilledRect2D(x0, y0, x1, y1, eventsColor[m]);
            DbgDraw::Text2D(x1 + DbgDraw::NormalCharW, y0, TEXT_COLOR, m.c_str());

            uint64 historyEventDuration = eventsHistory[m].second.crbegin()->first;
            sprintf(strbuf, "[%*d mcs]", ProfilerOverlayDetails::TRACE_LEGEND_DURATION_TEXT_WIDTH_CHARS - 6, historyEventDuration);
            DbgDraw::Text2D(x0 + legentWidth, y0, TEXT_COLOR, strbuf);

            y0 += ProfilerOverlayDetails::TRACE_LEGEND_ICON_SIZE + 1;
        }
    }

    //Draw separator
    int32 durationTextWidth = ProfilerOverlayDetails::TRACE_LEGEND_DURATION_TEXT_WIDTH_CHARS * DbgDraw::NormalCharW;
    x0 = drawRect.x + MARGIN + legentWidth + durationTextWidth + MARGIN;
    x1 = x0;
    y0 = drawRect.y + MARGIN + DbgDraw::NormalCharH + MARGIN;
    y1 = drawRect.y + drawRect.dy - MARGIN;
    DbgDraw::Line2D(x0, y0, x1, y1, LINE_COLOR);

    //Draw traces rects
    int32 x0trace = x0 + MARGIN;
    int32 y0trace = y0;
    int32 tracedx = drawRect.dx - x0trace - MARGIN;
    float32 dt = float32(tracedx) / (trace.maxTimestamp - trace.minTimestamp);

    for (const OverlayTrace::TraceRect& r : trace.rects)
    {
        x0 = x0trace + int32(r.start * dt);
        x1 = x0 + int32(r.duration * dt);
        y0 = y0trace + int32(r.depth * ProfilerOverlayDetails::TRACE_RECT_HEIGHT);
        y1 = y0 + ProfilerOverlayDetails::TRACE_RECT_HEIGHT;
        DbgDraw::FilledRect2D(x0, y0, x1, y1, r.color);
    }
}

void ProfilerOverlay::DrawHistory(const HistoryArray& history, const FastName& name, const Rect2i& rect)
{
    static const uint32 CHARTRECT_COLOR = rhi::NativeColorRGBA(0.f, 0.f, 1.f, .4f);
    static const uint32 CHART_COLOR = rhi::NativeColorRGBA(.5f, .11f, .11f, 1.f);
    static const uint32 CHART_FILTERED_COLOR = rhi::NativeColorRGBA(1.f, .18f, .18f, 1.f);
    static const uint32 TEXT_COLOR = rhi::NativeColorRGBA(1.f, 1.f, 1.f, 1.f);
    static const uint32 LINE_COLOR = rhi::NativeColorRGBA(.5f, 0.f, 0.f, 1.f);

    static const int32 MARGIN = ProfilerOverlayDetails::OVERLAY_RECT_MARGIN;
    static const int32 PADDING = ProfilerOverlayDetails::OVERLAY_RECT_PADDING;

    Rect2i drawRect(rect);
    drawRect.x += PADDING / 2;
    drawRect.y += PADDING / 2;
    drawRect.dx -= 2 * PADDING / 2;
    drawRect.dy -= 2 * PADDING / 2;

    Rect2i chartRect(drawRect);
    chartRect.x += MARGIN + ProfilerOverlayDetails::HISTORY_CHART_TEXT_COLUMN_WIDTH + MARGIN;
    chartRect.y += MARGIN + DbgDraw::NormalCharH;
    chartRect.dx -= ProfilerOverlayDetails::HISTORY_CHART_TEXT_COLUMN_WIDTH + 3 * MARGIN;
    chartRect.dy -= 2 * MARGIN + DbgDraw::NormalCharH;

    uint64 maxValue = 0;
    for (const HistoryInstance& h : history)
        maxValue = Max(maxValue, h.first);

    char strbuf[128];
    float32 ceilValue = float32((maxValue / ProfilerOverlayDetails::HISTORY_CHART_CEIL_STEP + 1) * ProfilerOverlayDetails::HISTORY_CHART_CEIL_STEP);

    DbgDraw::FilledRect2D(drawRect.x, drawRect.y, drawRect.x + drawRect.dx, drawRect.y + drawRect.dy, CHARTRECT_COLOR);

    DbgDraw::Line2D(chartRect.x, chartRect.y, chartRect.x, chartRect.y + chartRect.dy, LINE_COLOR);
    DbgDraw::Line2D(chartRect.x, chartRect.y + chartRect.dy, chartRect.x + chartRect.dx, chartRect.y + chartRect.dy, LINE_COLOR);

    const float32 chartstep = float32(chartRect.dx) / history.size();
    const float32 valuescale = chartRect.dy / ceilValue;

    const int32 chart0x = chartRect.x;
    const int32 chart0y = chartRect.y + chartRect.dy;

#define CHART_VALUE_HEIGHT(value) int32(value* valuescale)

    HistoryArray::const_iterator it = history.cbegin();
    int32 px = 0;
    int32 py = CHART_VALUE_HEIGHT(it->first);
    int32 pfy = CHART_VALUE_HEIGHT(it->second);
    ++it;

    int32 index = 1;
    HistoryArray::const_iterator hend = history.cend();
    for (; it != hend; ++it, ++index)
    {
        int32 x = int32(index * chartstep);
        int32 y = CHART_VALUE_HEIGHT(it->first);
        int32 fy = CHART_VALUE_HEIGHT(it->second);

        DbgDraw::Line2D(chart0x + px, chart0y - py, chart0x + x, chart0y - y, CHART_COLOR);
        DbgDraw::Line2D(chart0x + px, chart0y - pfy, chart0x + x, chart0y - fy, CHART_FILTERED_COLOR);

        px = x;
        py = y;
        pfy = fy;
    }

#undef CHART_VALUE_HEIGHT

    DbgDraw::Text2D(drawRect.x + MARGIN, drawRect.y + MARGIN, TEXT_COLOR, "\'%s\'", name.c_str());

    const int32 lastvalueIndent = (drawRect.dx - 2 * MARGIN) / DbgDraw::NormalCharW;
    sprintf(strbuf, "%lld [%.1f] mcs", history.crbegin()->first, history.crbegin()->second);
    DbgDraw::Text2D(drawRect.x + MARGIN, drawRect.y + MARGIN, TEXT_COLOR, "%*s", lastvalueIndent, strbuf);

    sprintf(strbuf, "%d mcs", int32(ceilValue));
    DbgDraw::Text2D(drawRect.x + MARGIN, drawRect.y + MARGIN + DbgDraw::NormalCharH, TEXT_COLOR, "%*s", ProfilerOverlayDetails::HISTORY_CHART_TEXT_COLUMN_CHARS, strbuf);
    DbgDraw::Text2D(drawRect.x + MARGIN, drawRect.y + drawRect.dy - MARGIN - DbgDraw::NormalCharH, TEXT_COLOR, "%*s", ProfilerOverlayDetails::HISTORY_CHART_TEXT_COLUMN_CHARS, "0 mcs");
}

int32 ProfilerOverlay::GetEnoughRectHeight(const OverlayTrace& trace)
{
    size_t legendCount = 0;
    for (const FastName& n : interestEventsNames)
        legendCount += trace.names.count(n);

    const int32 MARGIN = ProfilerOverlayDetails::OVERLAY_RECT_MARGIN;
    return MARGIN + DbgDraw::NormalCharH + MARGIN + int32(legendCount) * (DbgDraw::NormalCharH + 1) + MARGIN + ProfilerOverlayDetails::OVERLAY_RECT_PADDING;
}

}; //ns