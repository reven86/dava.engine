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
static uint32 frameToBuild = 0;
static uint32 frameToExecute = 0;
static uint32 framePoolSize = 0;
static DAVA::Spinlock frameSync;

void Initialize(uint32 _framePoolSize)
{
    framePoolSize = _framePoolSize;
    frames.resize(framePoolSize);
}

void RejectFrames()
{
    DAVA::LockGuard<DAVA::Spinlock> lock(frameSync);
    while (frameToExecute < frameToBuild)
    {
        DispatchPlatform::RejectFrame(frames[frameToExecute % framePoolSize]);
        frames[frameToExecute % framePoolSize].Reset();
        frameToExecute++;
    }
    if (frameToExecute >= framePoolSize)
    {
        frameToBuild -= framePoolSize;
        frameToExecute -= framePoolSize;
    }
}

void ProcessFrame()
{
    DVASSERT(framePoolSize);
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

            frames[frameToExecute].frameNumber = currentFrameNumber++;
            DispatchPlatform::ExecuteFrame(frames[frameToExecute]);
            frames[frameToExecute].Reset();
            frameToExecute++;
            frameSync.Lock();
            if (frameToExecute >= framePoolSize)
            {
                frameToBuild -= framePoolSize;
                frameToExecute -= framePoolSize;
            }
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
    bool frameValid = false;

    frameSync.Lock();
    uint32 frameSlot = frameToBuild % framePoolSize;
    if (frames[frameSlot].pass.size() != 0)
    {
        frames[frameSlot].readyToExecute = true;
        frames[frameSlot].sync = sync;
        frameToBuild++;
        frameValid = true;
    }
    frameSync.Unlock();
    DispatchPlatform::FinishFrame();
    return frameValid;
}

bool FrameReady()
{
    DAVA::LockGuard<DAVA::Spinlock> lock(frameSync);
    return frames[frameToExecute].readyToExecute;
}

uint32 FramesCount()
{
    DAVA::LockGuard<DAVA::Spinlock> lock(frameSync);
    return frameToBuild - frameToExecute;
}
void AddPass(Handle pass)
{
    DAVA::LockGuard<DAVA::Spinlock> lock(frameSync);
    frames[frameToBuild % framePoolSize].pass.push_back(pass);
    //frames.back().perfQuerySet = PerfQuerySet::Current();
}
}
}