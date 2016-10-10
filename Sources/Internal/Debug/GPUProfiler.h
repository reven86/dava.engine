#pragma once

#include "Base/BaseTypes.h"
#include "Render/RHI/rhi_Public.h"
#include "Debug/TraceEvent.h"
#include "Base/RingArray.h"

#define GPU_PROFILER_ENABLED 1

namespace DAVA
{
class GPUProfiler
{
public:
    GPUProfiler(uint32 framesCount = 180);
    ~GPUProfiler();

    struct MarkerInfo
    {
        const char* name;
        uint64 startTime;
        uint64 endTime;
    };

    struct FrameInfo
    {
        uint32 frameIndex = 0;
        uint64 startTime = 0;
        uint64 endTime = 0;
        Vector<MarkerInfo> markers;

        Vector<TraceEvent> GetTrace() const;
    };

    static GPUProfiler* const globalProfiler;

    const FrameInfo& GetLastFrame(uint32 index = 0) const;
    uint32 GetFramesCount() const;
    void OnFrameEnd(); //should be called before rhi::Present();

    Vector<TraceEvent> GetTrace();

    void AddMarker(rhi::HPerfQuery* query0, rhi::HPerfQuery* query1, const char* markerName);

    void Start();
    void Stop();
    bool IsStarted();

protected:
    struct PerfQueryPair
    {
        bool IsReady();
        void GetTimestamps(uint64& t0, uint64& t1);

        rhi::HPerfQuery query[2];
    };

    struct Marker
    {
        const char* name;
        PerfQueryPair perfQuery;
    };

    struct Frame
    {
        PerfQueryPair perfQuery;

        Vector<Marker> readyMarkers;
        List<Marker> pendingMarkers;

        uint32 globalFrameIndex = 0;

        void Reset();
    };

    void CheckPendingFrames();
    PerfQueryPair GetPerfQueryPair();
    void ResetPerfQueryPair(const PerfQueryPair& perfQuery);

    RingArray<FrameInfo> framesInfo;
    Vector<rhi::HPerfQuery> queryPool;
    List<Frame> pendingFrames;
    Frame currentFrame;

    bool profilerStarted = false;
};

} //ns DAVA

#if GPU_PROFILER_ENABLED

#define DAVA_GPU_PROFILER_PACKET(packet, marker_name) DAVA::GPUProfiler::globalProfiler->AddMarker(reinterpret_cast<rhi::HPerfQuery*>(&packet.perfQueryStart), reinterpret_cast<rhi::HPerfQuery*>(&packet.perfQueryEnd), marker_name);
#define DAVA_GPU_PROFILER_RENDER_PASS(passDesc, marker_name) DAVA::GPUProfiler::globalProfiler->AddMarker(reinterpret_cast<rhi::HPerfQuery*>(&passDesc.perfQueryStart), reinterpret_cast<rhi::HPerfQuery*>(&passDesc.perfQueryEnd), marker_name);

#else

#define DAVA_GPU_PROFILER_PACKET(packet, marker_name) 
#define DAVA_GPU_PROFILER_RENDER_PASS(pass, marker_name) 

#endif