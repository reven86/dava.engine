#include "Debug/GPUProfiler.h"
#include "Debug/DVAssert.h"
#include "Core/Core.h"
#include "Render/Renderer.h"
#include "Render/RHI/dbg_Draw.h"
#include <ostream>

//==============================================================================

namespace DAVA
{

#if GPU_PROFILER_ENABLED
static GPUProfiler GLOBAL_GPU_PROFILER;
GPUProfiler* const GPUProfiler::globalProfiler = &GLOBAL_GPU_PROFILER;
#else
GPUProfiler* const GPUProfiler::globalProfiler = nullptr;
#endif

static const char* GPU_FRAME_MARKER = "GPUFrame";
static const char* OVERLAY_MARKER = "GPUProfiler Overlay";
static const uint64 STATISTIC_GRAPH_CEIL_STEP = 500; //mcs

bool GPUProfiler::PerfQueryPair::IsReady()
{
    return (!query[0].IsValid() || rhi::PerfQueryIsReady(query[0])) && (!query[1].IsValid() || rhi::PerfQueryIsReady(query[1]));
}

void GPUProfiler::PerfQueryPair::GetTimestamps(uint64& t0, uint64& t1)
{
    t0 = query[0].IsValid() ? rhi::PerfQueryTimeStamp(query[0]) : 0;
    t1 = query[1].IsValid() ? rhi::PerfQueryTimeStamp(query[1]) : 0;
}

void GPUProfiler::FrameInfo::Dump(std::ostream& stream) const
{
    stream << "================================================" << std::endl;
    stream << "Frame " << frameIndex << ": " << endTime - startTime << " mcs" << std::endl;
    for (const MarkerInfo& m : markers)
    {
        stream << m.name << "[" << m.endTime - m.startTime << " mcs]" << std::endl;
    }
    stream << "================================================" << std::endl;
}

void GPUProfiler::FrameInfo::GetMarkerTimestamps(const char* markerName, uint64* ts0, uint64* ts1) const
{
    for (const MarkerInfo& m : markers)
    {
        if (markerName == m.name || (strcmp(markerName, m.name) == 0))
        {
            *ts0 = m.startTime;
            *ts1 = m.endTime;

            return;
        }
    }

    *ts0 = 0;
    *ts1 = 0;
}
void GPUProfiler::Frame::Reset()
{
    perfQuery = PerfQueryPair();

    readyMarkers.clear();
    pendingMarkers.clear();

    globalFrameIndex = 0;
}

GPUProfiler::GPUProfiler(uint32 _framesInfoCount)
    : framesInfoCount(_framesInfoCount)
{
    DVASSERT(framesInfoCount && "Should be > 0");
    framesInfo.resize(framesInfoCount);
}

GPUProfiler::~GPUProfiler()
{
}

const GPUProfiler::FrameInfo& GPUProfiler::GetLastFrame(uint32 index) const
{
    DVASSERT(index < framesInfoCount);
    return framesInfo[(framesInfoHead - index) % framesInfoCount];
}

uint32 GPUProfiler::GetFramesCount() const
{
    return framesInfoCount;
}

void GPUProfiler::CheckPendingFrames()
{
    List<Frame>::iterator fit = pendingFrames.begin();
    while (fit != pendingFrames.end())
    {
        Frame& frame = *fit;
        List<Marker>::iterator mit = frame.pendingMarkers.begin();
        while (mit != frame.pendingMarkers.end())
        {
            Marker& marker = *mit;
            if (marker.perfQuery.IsReady())
            {
                frame.readyMarkers.push_back(marker);
                mit = frame.pendingMarkers.erase(mit);
            }
            else
            {
                ++mit;
            }
        }

        if (!frame.pendingMarkers.size() && frame.perfQuery.IsReady())
        {
            FrameInfo frameInfo;
            frameInfo.frameIndex = frame.globalFrameIndex;
            frame.perfQuery.GetTimestamps(frameInfo.startTime, frameInfo.endTime);

            bool isFrameReliable = (frameInfo.startTime != rhi::NonreliableQueryValue) && (frameInfo.endTime != rhi::NonreliableQueryValue);
            for (Marker& marker : frame.readyMarkers)
            {
                if (isFrameReliable)
                {
                    uint64 t0, t1;
                    marker.perfQuery.GetTimestamps(t0, t1);

                    isFrameReliable = ((t0 != rhi::NonreliableQueryValue) && (t1 != rhi::NonreliableQueryValue));
                    if (isFrameReliable)
                    {
                        frameInfo.markers.push_back(MarkerInfo(marker.name, t0, t1));
                    }
                }

                ResetPerfQueryPair(marker.perfQuery);
            }
            frame.readyMarkers.clear();

            if (isFrameReliable)
            {
                framesInfoHead = (framesInfoHead + 1) % framesInfoCount;
                framesInfo[framesInfoHead] = frameInfo;
            }

            ResetPerfQueryPair(frame.perfQuery);

            fit = pendingFrames.erase(fit);
        }
        else
        {
            ++fit;
        }
    }
}

void GPUProfiler::OnFrameEnd()
{
    CheckPendingFrames();

    if (profilerStarted)
    {
        currentFrame.perfQuery = GetPerfQueryPair();
        currentFrame.globalFrameIndex = Core::Instance()->GetGlobalFrameIndex();
        rhi::SetFramePerfQueries(currentFrame.perfQuery.query[0], currentFrame.perfQuery.query[1]);
        pendingFrames.push_back(currentFrame);

        currentFrame.Reset();
    }

    if (overlayEnabled)
    {
        statisticMarkers.insert(statisticMarkers.begin(), GPU_FRAME_MARKER);
        statisticMarkers.push_back(OVERLAY_MARKER);

        ++statisticUpdateCounter;
        if (statisticUpdateCounter == statisticUpdatePeriod)
        {
            UpdateStatistic();
            statisticUpdateCounter = 0;
        }
        DrawStatistic();

        statisticMarkers.clear();
    }
}

void GPUProfiler::AddMarker(rhi::HPerfQuery* query0, rhi::HPerfQuery* query1, const char* markerName)
{
    if (profilerStarted)
    {
        PerfQueryPair p = GetPerfQueryPair();
        currentFrame.pendingMarkers.push_back(Marker(markerName, p));

        *query0 = p.query[0];
        *query1 = p.query[1];
    }
    else
    {
        *query0 = rhi::HPerfQuery();
        *query1 = rhi::HPerfQuery();
    }

    if (overlayEnabled)
    {
        statisticMarkers.push_back(markerName);
    }
}

void GPUProfiler::EnableStatisticOverlay(uint32 updatePeriod)
{
    overlayEnabled = true;
    statisticUpdatePeriod = updatePeriod ? updatePeriod : 1;
    statisticUpdateCounter = 0;
}

void GPUProfiler::DisableStatisticOverlay()
{
    overlayEnabled = false;
    statisticUpdateCounter = 0;
}

void GPUProfiler::Start()
{
    profilerStarted = true;
}

void GPUProfiler::Stop()
{
    profilerStarted = false;
}

GPUProfiler::PerfQueryPair GPUProfiler::GetPerfQueryPair()
{
    PerfQueryPair p;
    for (rhi::HPerfQuery& q : p.query)
    {
        if (queryPool.size())
        {
            q = queryPool.back();
            queryPool.pop_back();
        }
        else
        {
            if (rhi::DeviceCaps().isPerfQuerySupported)
                q = rhi::CreatePerfQuery();
        }
    }

    return p;
}

void GPUProfiler::ResetPerfQueryPair(const GPUProfiler::PerfQueryPair& perfQuery)
{
    for (rhi::HPerfQuery q : perfQuery.query)
    {
        if (q.IsValid())
        {
            rhi::ResetPerfQuery(q);
            queryPool.push_back(q);
        }
    }
}

void GPUProfiler::UpdateStatistic()
{
    for (auto& m : statistic)
    {
        m.second.first.reserve(framesInfoCount);
        m.second.first.clear();
        m.second.second = 0;
    }

    uint64 t0, t1, deltaTime;
    for (uint32 fi = 0; fi < framesInfoCount; ++fi)
    {
        const FrameInfo& frameInfo = framesInfo[(framesInfoHead + fi + 1) % framesInfoCount];
        for (const char* n : statisticMarkers)
        {
            if (n == GPU_FRAME_MARKER)
            {
                deltaTime = frameInfo.endTime - frameInfo.startTime;
            }
            else
            {
                frameInfo.GetMarkerTimestamps(n, &t0, &t1);
                deltaTime = t1 - t0;
            }

            statistic[n].first.push_back(deltaTime);
            statistic[n].second = Max(statistic[n].second, deltaTime);
        }
    }
}

void GPUProfiler::DrawStatistic()
{
    if (!statistic.size())
        return;

    static const uint32 STATISTIC_GRAPHRECT_COLOR = rhi::NativeColorRGBA(0.f, 0.f, 1.f, .3f);
    static const uint32 STATISTIC_GRAPH_COLOR = rhi::NativeColorRGBA(1.f, 0.f, 0.f, 1.f);
    static const uint32 STATISTIC_TEXT_COLOR = rhi::NativeColorRGBA(1.f, 1.f, 1.f, 1.f);
    static const uint32 STATISTIC_LINE_COLOR = rhi::NativeColorRGBA(.5f, 0.f, 0.f, 1.f);

    DbgDraw::EnsureInited();

    DbgDraw::SetScreenSize(uint32(Renderer::GetFramebufferWidth()), uint32(Renderer::GetFramebufferHeight()));

    DbgDraw::SetNormalTextSize();
    const int32 rectmargin = 3;
    const int32 rectpadding = 8;
    Rect2i graphrect(rectpadding, rectpadding, Renderer::GetFramebufferWidth() / 3, 150);
    const int32 textcolumnchars = 9;

    const int32 textcolumn = DbgDraw::NormalCharW * textcolumnchars;
    const int32 graphdx = graphrect.dx - textcolumn - 3 * rectmargin;
    const int32 graphdy = graphrect.dy - rectmargin * 2 - DbgDraw::NormalCharH;
    char strbuf[128];
    for (const char* n : statisticMarkers)
    {
        const int32 graph0x = graphrect.x + rectmargin + textcolumn + rectmargin;
        const int32 graph0y = graphrect.y + graphrect.dy - rectmargin;

        uint64 maxValue = statistic[n].second;
        float32 ceilValue = float32((maxValue / STATISTIC_GRAPH_CEIL_STEP + 1) * STATISTIC_GRAPH_CEIL_STEP);

        DbgDraw::FilledRect2D(graphrect.x, graphrect.y, graphrect.x + graphrect.dx, graphrect.y + graphrect.dy, STATISTIC_GRAPHRECT_COLOR);

        DbgDraw::Line2D(graph0x, graph0y, graph0x, graphrect.y + rectmargin + DbgDraw::NormalCharH, STATISTIC_LINE_COLOR);
        DbgDraw::Line2D(graph0x, graph0y, graphrect.x + graphrect.dx - rectmargin, graph0y, STATISTIC_LINE_COLOR);

        const Vector<uint64>& values = statistic[n].first;
        const uint32 valuesCount = uint32(values.size());
        if (!valuesCount)
            continue;

        const float32 graphstep = float32(graphdx) / valuesCount;
        int32 prevValue = int32(graphdy * values[0] / ceilValue);
        for (uint32 v = 1; v < valuesCount; ++v)
        {
            int32 y = int32(graphdy * values[v] / ceilValue);
            DbgDraw::Line2D(graph0x + int32((v - 1) * graphstep), graph0y - prevValue, graph0x + int32(v * graphstep), graph0y - y, STATISTIC_GRAPH_COLOR);
            prevValue = y;
        }

        const int32 graphChars = (graphrect.dx - 2 * rectmargin) / DbgDraw::NormalCharW;
        DbgDraw::Text2D(graphrect.x + rectmargin, graphrect.y + rectmargin, STATISTIC_TEXT_COLOR, "\'%s\'", n);
        sprintf(strbuf, "%lld mcs", values.back());
        DbgDraw::Text2D(graphrect.x + rectmargin, graphrect.y + rectmargin, STATISTIC_TEXT_COLOR, "%*s", graphChars, strbuf);
        sprintf(strbuf, "%d mcs", int32(ceilValue));
        DbgDraw::Text2D(graphrect.x + rectmargin, graphrect.y + rectmargin + DbgDraw::NormalCharH, STATISTIC_TEXT_COLOR, "%*s", textcolumnchars, strbuf);
        DbgDraw::Text2D(graphrect.x + rectmargin, graphrect.y + graphrect.dy - rectmargin - DbgDraw::NormalCharH, STATISTIC_TEXT_COLOR, "%*s", textcolumnchars, "0 mcs");

        graphrect.y += graphrect.dy + rectmargin;
        if (graphrect.y + graphrect.dy > Renderer::GetFramebufferHeight())
        {
            graphrect.y = rectpadding;
            graphrect.x += graphrect.dx + rectpadding;
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
    AddMarker(reinterpret_cast<rhi::HPerfQuery*>(&passConfig.perfQueryStart), reinterpret_cast<rhi::HPerfQuery*>(&passConfig.perfQueryEnd), OVERLAY_MARKER);

    rhi::HPacketList packetList;
    rhi::HRenderPass pass = rhi::AllocateRenderPass(passConfig, 1, &packetList);
    rhi::BeginRenderPass(pass);
    rhi::BeginPacketList(packetList);

    DbgDraw::FlushBatched(packetList);

    rhi::EndPacketList(packetList);
    rhi::EndRenderPass(pass);
}
}