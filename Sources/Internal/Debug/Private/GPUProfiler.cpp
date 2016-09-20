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
}

void GPUProfiler::DumpJSON(std::ostream& stream) const
{
    stream << "{ \"traceEvents\": [" << std::endl;

    for (uint32 fi = 0; fi < framesInfoCount; ++fi)
    {
        const FrameInfo& frameInfo = framesInfo[(framesInfoHead + fi + 1) % framesInfoCount];

        stream << "{ \"pid\":0, \"tid\":0, \"ts\":" << frameInfo.startTime << ", \"ph\":\"B\", \"cat\":\"\", \"name\":\"frame " << frameInfo.frameIndex << "\" }," << std::endl;
        stream << "{ \"pid\":0, \"tid\":0, \"ts\":" << frameInfo.endTime << ", \"ph\":\"E\", \"cat\":\"\", \"name\":\"frame " << frameInfo.frameIndex << "\" }";

        for (const MarkerInfo& m : frameInfo.markers)
        {
            stream << "," << std::endl;
            stream << "{ \"pid\":0, \"tid\":0, \"ts\":" << m.startTime << ", \"ph\":\"B\", \"cat\":\"\", \"name\":\"" << m.name << "\" }," << std::endl;
            stream << "{ \"pid\":0, \"tid\":0, \"ts\":" << m.endTime << ", \"ph\":\"E\", \"cat\":\"\", \"name\":\"" << m.name << "\" }";
        }

        if (fi < (framesInfoCount - 1))
            stream << ",";
        stream << std::endl;
    }

    stream << "] }" << std::endl;

    stream.flush();
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

}