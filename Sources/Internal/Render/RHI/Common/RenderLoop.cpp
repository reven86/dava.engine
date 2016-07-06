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
static DAVA::Atomic<bool> renderThreadSuspendSyncReached = false;
static DAVA::Atomic<bool> renderThreadSuspended = false;

static DAVA::Atomic<bool> renderThreadExitPending = false;
}

namespace DispatchPlatform
{
void (*InitContext)() = nullptr;
void (*AcquireContext)() = nullptr;
void (*ReleaseContext)() = nullptr;
void (*CheckSurface)() = nullptr;

void (*Suspend)() = nullptr;

void (*ProcessImmediateCommands)() = nullptr;

RenderPassBase* (*GetPass)(Handle passHandle) = nullptr;
SyncObjectBase* (*GetSyncObject)(Handle passHandle) = nullptr;
}

using namespace Details;
void Present(Handle syncHandle) // called from main thread
{
    TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "rhi::present");
    Trace("rhi-gl.present\n");

    uint32 frame_cnt = 0;
    FrameLoop::frameSync.Lock();
    {
        frame_cnt = FrameLoop::frames.size();
        if (frame_cnt)
        {
            FrameLoop::frames.back().readyToExecute = true;
            FrameLoop::frames.back().sync = syncHandle;
            /*??????? do we really need frame started?*/ //_GLES2_FrameStarted = false;
        }
    }
    FrameLoop::frameSync.Unlock();

    if (!frame_cnt)
    {
        if (syncHandle != InvalidHandle) //frame is empty - still need to sync if required
        {
            SyncObjectBase* syncObject = DispatchPlatform::GetSyncObject(syncHandle);
            syncObject->is_signaled = true;
        }
        return;
    }

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
            FrameLoop::frameSync.Lock();
            frame_cnt = static_cast<uint32>(FrameLoop::frames.size());
            FrameLoop::frameSync.Unlock();

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
        bool do_wait = true;
        while (do_wait)
        {
            if (renderThreadExitPending)
                break;

            DispatchPlatform::ProcessImmediateCommands();

            FrameLoop::frameSync.Lock();
            do_wait = !(FrameLoop::frames.size() && FrameLoop::frames.begin()->readyToExecute) && !renderThreadSuspended.Get();
            FrameLoop::frameSync.Unlock();

            if (do_wait)
            {
                framePreparedEvent.Wait();
            }
        }
        TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "renderer_wait_core");

        if (!renderThreadExitPending)
        {
            FrameLoop::ProcessFrame();
        }

        frameDoneEvent.Signal();

        TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "rhi::render_loop");
    }

    Logger::Info("[RHI] render-thread finished");
}

void InitializeRenderThread(uint32 frameCount)
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

//------------------------------------------------------------------------------

void UninitializeRenderThread()
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

//------------------------------------------------------------------------------

void SuspendRender()
{
    if (renderThreadFrameCount)
    {
        renderThreadSuspended.Set(true);
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

//------------------------------------------------------------------------------

void ResumeRender()
{
    Logger::Error("Render Resumed");
    if (renderThreadFrameCount)
    {
        renderThreadSuspended.Set(false);
        renderThreadSuspendSync.Post();
    }
}
}
}