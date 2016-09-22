#include "FrameLoop.h"
#include "rhi_Pool.h"
#include "../rhi_Public.h"
#include "rhi_Private.h"
#include "rhi_CommonImpl.h"
#include "Debug/CPUProfiler.h"
#include "Concurrency/Thread.h"

namespace rhi
{
namespace FrameLoop
{
static uint32 currFrameNumber = 0;
static DAVA::Vector<CommonImpl::Frame> frames;
static uint32 frameCount = 0;
static DAVA::Spinlock frameSync;
static bool frameStarted = false;

void RejectFrames()
{
    frameSync.Lock();
    for (std::vector<CommonImpl::Frame>::iterator f = frames.begin(); f != frames.end();)
    {
        if (f->readyToExecute)
        {
            DispatchPlatform::RejectFrame(std::move(*f));
            f = frames.erase(f);
            frameCount--;
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
    bool presentResult = false;
    if (NeedRestoreResources())
    {
        RejectFrames();
        presentResult = true;
    }
    else
    {
        {
            DAVA_CPU_PROFILER_SCOPE("rhi::ExecuteFrame");
            frameSync.Lock();
            CommonImpl::Frame currFrame = std::move(frames.front());
            frames.erase(frames.begin());
            frameSync.Unlock();
            currFrame.frameNumber = currFrameNumber++;
            DispatchPlatform::ExecuteFrame(std::move(currFrame));

            frameSync.Lock();
            frameCount--;
            frameSync.Unlock();
        }

        {
            DAVA_CPU_PROFILER_SCOPE("SwapChain::Present");
            presentResult = DispatchPlatform::PresntBuffer();
        }
    }

    if (!presentResult)
    {
        RejectFrames();
        DispatchPlatform::ResetBlock();
    }
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
    frameStarted = false;
    frameSync.Unlock();
    DispatchPlatform::FinishFrame();
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
    frameSync.Lock();
    uint32 frame_cnt = frameCount;
    frameSync.Unlock();
    return frame_cnt;
}
void AddPass(Handle pass)
{
    frameSync.Lock();
    if (!frameStarted)
    {
        frames.push_back(CommonImpl::Frame());
        frameCount++;
        frameStarted = true;
    }
    frames.back().pass.push_back(pass);
    //frames.back().perfQuerySet = PerfQuerySet::Current();
    frameSync.Unlock();
}
}
}