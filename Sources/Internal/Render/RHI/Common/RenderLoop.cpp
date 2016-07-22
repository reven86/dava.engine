#include "rhi_Pool.h"
#include "CommonImpl.h"
#include "FrameLoop.h"
#include "../rhi_Type.h"
#include "Concurrency/AutoResetEvent.h"
/*#include "../Common/rhi_Private.h"
#include "../Common/rhi_RingBuffer.h"
#include "../Common/dbg_StatSet.h"*/

namespace rhi
{
namespace RenderLoop
{
namespace Details
{
static DAVA::AutoResetEvent framePreparedEvent(false, 400);
static DAVA::AutoResetEvent frameDoneEvent(false, 400);
static DAVA::Thread* renderThread = nullptr;
static uint32 renderThreadFrameCount = 0;
static DAVA::Semaphore renderThredStartedSync(1);

static DAVA::Semaphore renderThreadSuspendSync;
static DAVA::Atomic<bool> renderThreadSuspendSyncReached(false);
static DAVA::Atomic<bool> renderThreadSuspended(false);

static DAVA::Atomic<bool> renderThreadExitPending(false);
}

using namespace Details;


void Present(Handle syncHandle) // called from main thread
{
    TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "rhi::present");
    Trace("rhi-gl.present\n");

    bool res = FrameLoop::FinishFrame(syncHandle);
    if (!res) //if present was called without actual work - need to do nothing here (or should we swap buffers in any case?)
        return;

    uint32 frame_cnt = 0;
    if (renderThreadFrameCount == 0) //single thread render
    {
        FrameLoop::ProcessFrame();
    }
    else //wait for render thread if needed
    {
        if (!renderThreadSuspended)
            framePreparedEvent.Signal();

        TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "core_wait_renderer");
        do
        {
            frame_cnt = FrameLoop::FramesCount();
            if (frame_cnt >= renderThreadFrameCount)
                frameDoneEvent.Wait();

        } while (frame_cnt >= renderThreadFrameCount);

        TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "core_wait_renderer");
    }

    TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "rhi::present");
}

//------------------------------------------------------------------------------

static void RenderFunc(DAVA::BaseObject* obj, void*, void*)
{
    DispatchPlatform::InitContext();

    renderThredStartedSync.Post();
    Logger::Info("[RHI] render-thread started");

    while (!renderThreadExitPending)
    {
        TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "rhi::render_loop");

        if (renderThreadSuspended.Get())
        {
            DispatchPlatform::Suspend();
            renderThreadSuspendSyncReached = true;
            renderThreadSuspendSync.Wait();
        }

        DispatchPlatform::CheckSurface();

        TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "renderer_wait_core");
        bool frameReady = false;
        while (!frameReady)
        {
            //exit or suspend should leave frame loop
            if (renderThreadExitPending || renderThreadSuspended.Get())
                break;

            DispatchPlatform::ProcessImmediateCommands();
            frameReady = FrameLoop::FrameReady();

            if (!frameReady)
            {
                framePreparedEvent.Wait();
            }
        }
        TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "renderer_wait_core");

        if (frameReady)
        {
            FrameLoop::ProcessFrame();
            frameDoneEvent.Signal();
        }

        TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "rhi::render_loop");
    }

    Logger::Info("[RHI] render-thread finished");
}

void InitializeRenderLoop(uint32 frameCount)
{
    renderThreadFrameCount = frameCount;
    DVASSERT(DispatchPlatform::InitContext);
    DVASSERT(DispatchPlatform::AcquireContext);
    DVASSERT(DispatchPlatform::ReleaseContext);

    if (renderThreadFrameCount) //?ASSERT
    {
        DispatchPlatform::ReleaseContext();

        renderThread = DAVA::Thread::Create(DAVA::Message(&RenderFunc));
        renderThread->SetName("RHI.RENDER_THREAD");
        renderThread->Start();
        //        renderThread->SetPriority(DAVA::Thread::PRIORITY_HIGH); //think - may be threading priority should be somehow configurable
        renderThredStartedSync.Wait();
    }
}

void SuspendRender()
{
    DVASSERT(!renderThreadSuspended);
    renderThreadSuspended.Set(true);
    if (renderThreadFrameCount)
    {
        while (!renderThreadSuspendSyncReached)
        {
            framePreparedEvent.Signal(); //avoid stall
        }
        renderThreadSuspendSyncReached = false;
    }
    else
    {
        DispatchPlatform::Suspend();
    }

    Logger::Error("Render  Suspended");
}

void ResumeRender()
{
    DVASSERT(renderThreadSuspended);
    Logger::Error("Render Resumed");
    renderThreadSuspended.Set(false);
    if (renderThreadFrameCount)
    {
        renderThreadSuspendSync.Post();
    }
}

void UninitializeRenderLoop()
{
    if (renderThreadFrameCount) //?ASSERT
    {
        renderThreadExitPending = true;
        if (renderThreadSuspended.Get())
        {
            Logger::Info("RenderThread is suspended. Need resume it to be able to join.");
            ResumeRender();
        }

        framePreparedEvent.Signal();

        Logger::Info("UninitializeRenderThread join begin");
        renderThread->Join();
        Logger::Info("UninitializeRenderThread join end");
    }
}
}
}