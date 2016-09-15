#pragma once

#include "Base/BaseTypes.h"
#include "Render/RHI/rhi_Public.h"
#include <iosfwd>

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
        MarkerInfo(const char* _name, uint64 start, uint64 end)
            : name(_name)
            , startTime(start)
            , endTime(end)
        {
        }

        const char* name = nullptr;
        uint64 startTime = 0;
        uint64 endTime = 0;
    };

    struct FrameInfo
    {
        uint32 frameIndex = 0;
        uint64 startTime = 0;
        uint64 endTime = 0;
        Vector<MarkerInfo> markers;

        void Dump(std::ostream& stream) const;
        void GetMarkerTimestamps(const char* markerName, uint64* ts0, uint64* ts1) const;
    };

    static GPUProfiler* const globalProfiler;

    const FrameInfo& GetLastFrame(uint32 index = 0) const;
    uint32 GetFramesCount() const;
    void OnFrameEnd(); //should be called before rhi::Present();

    void DumpJSON(std::ostream& stream) const;

    void AddMarker(rhi::HPerfQuery* query0, rhi::HPerfQuery* query1, const char* markerName);

    void EnableStatisticOverlay(uint32 updatePeriod = 5);
    void DisableStatisticOverlay();

    void Start();
    void Stop();

protected:
    struct PerfQueryPair
    {
        bool IsReady();
        void GetTimestamps(uint64& t0, uint64& t1);

        rhi::HPerfQuery query[2];
    };

    struct Marker
    {
        Marker(const char* _name, const PerfQueryPair& _perfQuery)
            : name(_name)
            , perfQuery(_perfQuery)
        {
        }

        const char* name = nullptr;
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

    void UpdateStatistic();
    void DrawStatistic();

    Vector<FrameInfo> framesInfo;
    uint32 framesInfoCount = 0;
    uint32 framesInfoHead = 0;

    Vector<rhi::HPerfQuery> queryPool;
    List<Frame> pendingFrames;
    Frame currentFrame;

    Map<const char*, std::pair<Vector<uint64>, uint64>> statistic; //[marker name]: (values, max-value)
    Vector<const char*> statisticMarkers;
    uint32 statisticUpdatePeriod = 0;
    uint32 statisticUpdateCounter = 0;
    bool overlayEnabled = false;

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