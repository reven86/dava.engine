#include "FrameLoop.h"
#include "rhi_Pool.h"
#include "CommonImpl.h"
#include "Debug/Profiler.h"
#include "Concurrency/Thread.h"

namespace rhi
{
namespace FrameLoop
{
static uint32 currFrameNumber = 0;
static DAVA::Vector<CommonImpl::Frame> frames;
static DAVA::Spinlock frameSync;
static bool frameStarted = false;
static bool renderContextReady = false;
static bool resetPending = false;

void RejectFrames()
{
    frameSync.Lock();
    for (std::vector<CommonImpl::Frame>::iterator f = frames.begin(); f != frames.end();)
    {
        if (f->readyToExecute)
        {
            DispatchPlatform::RejectFrame(std::move(*f));
            f = frames.erase(f);
        }
        else
        {
            ++f;
        }
    }
    frameSync.Unlock();
}

void ProcessFrame()
{
    TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "ProcessFrame");

    if (!renderContextReady) //no render context - just reject frames and do nothing;
    {
        RejectFrames();
        return;
    }

    bool presentResult = false;
    if (!resetPending)
    {
        TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "ExecuteFrameCommands");
        currFrameNumber++;

        frameSync.Lock();
        CommonImpl::Frame currFrame = std::move(frames.front());
        frames.erase(frames.begin());
        frameSync.Unlock();

        currFrame.frameNumber = currFrameNumber;
        DispatchPlatform::ExecuteFrame(std::move(currFrame));
        TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "ExecuteFrameCommands");

        TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "PresntBuffer");
        presentResult = DispatchPlatform::PresntBuffer();
        TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "PresntBuffer");
    }

    if (!presentResult)
    {
        RejectFrames();
        DispatchPlatform::ResetBlock();
    }

    TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "ProcessFrame");
}

bool FinishFrame(Handle sync)
{
    size_t frame_cnt = 0;

    frameSync.Lock();
    frame_cnt = frames.size();
    if (frame_cnt)
    {
        frames.back().readyToExecute = true;
        frames.back().sync = sync;
    }
    frameSync.Unlock();

    frameStarted = false;

    return frame_cnt != 0;
}

bool FrameReady()
{
    frameSync.Lock();
    bool res = (frames.size() && frames.begin()->readyToExecute);
    frameSync.Unlock();
    return res;
}

uint32 FramesCount()
{
    FrameLoop::frameSync.Lock();
    uint32 frame_cnt = static_cast<uint32>(frames.size());
    FrameLoop::frameSync.Unlock();
    return frame_cnt;
}
void AddPass(Handle pass, uint32 priority)
{
    frameSync.Lock();
    if (!frameStarted)
    {
        frames.push_back(CommonImpl::Frame());
        frameStarted = true;
        DispatchPlatform::InvalidateFrameCache();
    }
    frames.back().pass.push_back(pass);
}
}
}