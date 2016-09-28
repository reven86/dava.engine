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
static uint32 currentFrameNumber = 0;
static DAVA::Vector<CommonImpl::Frame> frames;
static uint32 frameCount = 0;
static DAVA::Spinlock frameSync;
static bool frameStarted = false;

void RejectFrames()
{
    DAVA::LockGuard<DAVA::Spinlock> lock(frameSync);
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
            currFrame.frameNumber = currentFrameNumber++;
            DispatchPlatform::ExecuteFrame(std::move(currFrame));

            frameSync.Lock();
            frameCount--;
            frameSync.Unlock();
        }

        {
            DAVA_CPU_PROFILER_SCOPE("SwapChain::Present");
            presentResult = DispatchPlatform::PresentBuffer();
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
    DAVA::LockGuard<DAVA::Spinlock> lock(frameSync);
    return (frames.size() && frames.begin()->readyToExecute);
}

uint32 FramesCount()
{
    DAVA::LockGuard<DAVA::Spinlock> lock(frameSync);
    return frameCount;
}
void AddPass(Handle pass)
{
    DAVA::LockGuard<DAVA::Spinlock> lock(frameSync);
    if (!frameStarted)
    {
        frames.push_back(CommonImpl::Frame());
        frameCount++;
        frameStarted = true;
    }
    frames.back().pass.push_back(pass);
    //frames.back().perfQuerySet = PerfQuerySet::Current();
}
}
}