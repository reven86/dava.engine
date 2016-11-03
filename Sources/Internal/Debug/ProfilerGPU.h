#pragma once

#include "Base/BaseTypes.h"
#include "Render/RHI/rhi_Public.h"
#include "Debug/TraceEvent.h"
#include "Debug/Private/RingArray.h"

#define PROFILER_GPU_ENABLED 1

namespace DAVA
{
struct ProfilerGPUDetails;

/**
    \ingroup profilers
    \brief   Profiler allows measure executing time of render-commands on hardware.
    \details Profiler allows measure executing time of render-commands on hardware.
             Using this profiler you can mark 'rhi::Packet', 'rhi::RenderPassDescriptor' or 'DAVA::RenderBatch' to trace how much time took executing corresponding render-commands.
             Same information you can receive about whole frame using 'FrameInfo'.
             Identification of markers become by string-name.

             To mark packet, render-pass or render-batch use special defines:
               - DAVA_PROFILER_GPU_PACKET(packet, marker_name)         -- Add marker with marker_name for single packet.
               - DAVA_PROFILER_GPU_RENDER_PASS(passDesc, marker_name)  -- Add marker with marker_name for whole render-pass.
               - DAVA_PROFILER_GPU_RENDER_BATCH(batch, marker_name)    -- Add marker with marker_name for single render-batch.
*/
class ProfilerGPU
{
public:
    static const FastName TRACE_ARG_FRAME; ///< Name of frame index argument of generated TraceEvent

    struct MarkerInfo
    {
        const char* name; ///< Marker name
        uint64 startTime; ///< Marker start timestamp, in microseconds
        uint64 endTime; ///< Marker end timestamp, in microseconds
    };

    struct FrameInfo
    {
        uint32 frameIndex = 0; ///< Index of executed frame
        uint64 startTime = 0; ///< Timestamp of start frame execution
        uint64 endTime = 0; ///< Timestamp of end frame execution
        Vector<MarkerInfo> markers; ///< Marker that was added in this frame

        /**
            Build trace of frame
            \return Vector of trace events
        */
        Vector<TraceEvent> GetTrace() const;
    };

    static ProfilerGPU* const globalProfiler; ///< Global Engine Profiler

    /**
        Return information about executed frames on GPU. Index counting from the end. Index equals zero means executed last frame, index equal one - penultimate frame, etc.
        \param[in] index Frame index from the end 
        \return frame information about requested frame
    */
    const FrameInfo& GetFrame(uint32 index = 0) const;

    /**
        \return count of available executed frames
    */
    uint32 GetFramesCount() const;

    /**
        Frame separator. You should call this method once per-frame and before 'rhi::Present()'.
    */
    void OnFrameEnd();

    /**
        Build trace of all available frames. Trace can be dumped to JSON Chromium Trace Viewer format.
        \return Vector of trace events
    */
    Vector<TraceEvent> GetTrace();

    /**
        Add marker to track render-commands executing. Use DAVA_PROFILER_GPU_* defines insted direct call.

        \param[out] query0 PerfQuery to retrieve start timestamp
        \param[out] query1 PerfQuery to retrieve end timestamp
        \param[in] markerName Marker name
    */
    void AddMarker(rhi::HPerfQuery* query0, rhi::HPerfQuery* query1, const char* markerName);

    /**
        Start time measuring
    */
    void Start();

    /**
        Stop time measuring
    */
    void Stop();

    /**
        \return Is time measuring started
    */
    bool IsStarted();

protected:
    ProfilerGPU(uint32 framesCount = 180);
    ~ProfilerGPU();

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

    friend struct ProfilerGPUDetails;
};

} //ns DAVA

#if PROFILER_GPU_ENABLED

#define DAVA_PROFILER_GPU_PACKET(packet, marker_name) DAVA::ProfilerGPU::globalProfiler->AddMarker(reinterpret_cast<rhi::HPerfQuery*>(&packet.perfQueryStart), reinterpret_cast<rhi::HPerfQuery*>(&packet.perfQueryEnd), marker_name);
#define DAVA_PROFILER_GPU_RENDER_PASS(passDesc, marker_name) DAVA::ProfilerGPU::globalProfiler->AddMarker(reinterpret_cast<rhi::HPerfQuery*>(&passDesc.perfQueryStart), reinterpret_cast<rhi::HPerfQuery*>(&passDesc.perfQueryEnd), marker_name);
#define DAVA_PROFILER_GPU_RENDER_BATCH(batch, marker_name) DAVA::ProfilerGPU::globalProfiler->AddMarker(reinterpret_cast<rhi::HPerfQuery*>(&batch->perfQueryStart), reinterpret_cast<rhi::HPerfQuery*>(&batch->perfQueryEnd), marker_name);

#else

#define DAVA_PROFILER_GPU_PACKET(packet, marker_name) 
#define DAVA_PROFILER_GPU_RENDER_PASS(pass, marker_name) 
#define DAVA_PROFILER_GPU_RENDER_BATCH(batch, marker_name) 

#endif