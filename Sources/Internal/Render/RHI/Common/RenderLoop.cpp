#include "RenderLoop.h"
#include "rhi_Pool.h"
#include "rhi_CommonImpl.h"
#include "FrameLoop.h"
#include "rhi_Private.h"
#include "../rhi_Type.h"
#include "Concurrency/AutoResetEvent.h"
#include "Concurrency/Concurrency.h"
#include "Debug/ProfilerCPU.h"
#include "Debug/ProfilerMarkerNames.h"
#include "Logger/Logger.h"
#include <atomic>

using DAVA::Logger;

namespace rhi
{
namespace RenderLoop
{
static DAVA::AutoResetEvent framePreparedEvent(false, 400);
static DAVA::AutoResetEvent frameDoneEvent(false, 400);
static DAVA::AutoResetEvent resetDoneEvent(false, 400);
static DAVA::Thread* renderThread = nullptr;
static uint32 renderThreadFrameCount = 0;
static DAVA::Semaphore renderThredStartedSync;

static DAVA::Semaphore renderThreadSuspendSync;
static std::atomic<bool> renderThreadSuspendSyncReached(false);
static std::atomic<bool> renderThreadSuspended(false);

static std::atomic<bool> renderThreadExitPending(false);

static std::atomic<bool> resetPending(false);

static std::atomic<CommonImpl::ImmediateCommand*> pendingImmediateCmd(nullptr);
static DAVA::Mutex pendingImmediateCmdSync;

struct ScheduledDeleteResource
{
    Handle handle;
    ResourceType resourceType;
};
const static uint32 frameSyncObjectsCount = 16;
static uint32 currFrameSyncId = 0;
static DAVA::Array<HSyncObject, frameSyncObjectsCount> frameSyncObjects;
static DAVA::Array<std::vector<ScheduledDeleteResource>, frameSyncObjectsCount> scheduledDeleteResources;
static DAVA::Spinlock scheduledDeleteMutex;
static void ProcessScheduledDelete();

void Present() // called from main thread
{
    DAVA_PROFILER_CPU_SCOPE(DAVA::ProfilerCPUMarkerName::RHI_PRESENT);

    scheduledDeleteMutex.Lock();
    if (scheduledDeleteResources[currFrameSyncId].size() && !frameSyncObjects[currFrameSyncId].IsValid())
        frameSyncObjects[currFrameSyncId] = CreateSyncObject();
    Handle frameSync = frameSyncObjects[currFrameSyncId];
    currFrameSyncId = (currFrameSyncId + 1) % frameSyncObjectsCount;
    DVASSERT(scheduledDeleteResources[currFrameSyncId].empty()); //we are not going to mix new resources for deletion with existing once still waiting
    DVASSERT(!frameSyncObjects[currFrameSyncId].IsValid());
    scheduledDeleteMutex.Unlock();

    bool validFrame = FrameLoop::FinishFrame(frameSync);

    if (!validFrame) //if present was called without actual work - need to do nothing here (or should we swap buffers in any case?)
    {
        Logger::Debug(" *** empty frame finished **");
        return;
    }

    uint32 frameCnt = 0;
    if (renderThreadFrameCount == 0) //single thread render
    {
        if (renderThreadSuspended)
            FrameLoop::RejectFrames();
        else
            FrameLoop::ProcessFrame();
    }
    else //wait for render thread if needed
    {
        DAVA_PROFILER_CPU_SCOPE(DAVA::ProfilerCPUMarkerName::RHI_WAIT_FRAME_EXECUTION);

        if (!renderThreadSuspended)
            framePreparedEvent.Signal();

        do
        {
            frameCnt = FrameLoop::FramesCount();
            if (frameCnt >= renderThreadFrameCount)
                frameDoneEvent.Wait();

        } while (frameCnt >= renderThreadFrameCount);
    }

    ProcessScheduledDelete();
}

//------------------------------------------------------------------------------

static void RenderFunc()
{
    DispatchPlatform::InitContext();

    renderThredStartedSync.Post();
    Logger::Info("[RHI] render-thread started");

    while (!renderThreadExitPending)
    {
        DAVA_PROFILER_CPU_SCOPE(DAVA::ProfilerCPUMarkerName::RHI_RENDER_LOOP);
        if (renderThreadSuspended.load())
        {
            DispatchPlatform::FinishRendering();
            renderThreadSuspendSyncReached = true;
            renderThreadSuspendSync.Wait();
            DispatchPlatform::ValidateSurface();
        }
        bool frameReady = false;
        {
            DAVA_PROFILER_CPU_SCOPE(DAVA::ProfilerCPUMarkerName::RHI_WAIT_FRAME_CONSTRUCTION);

            while ((!frameReady) && (!resetPending))
            {
                //exit or suspend should leave frame loop
                if (renderThreadExitPending || renderThreadSuspended.load(std::memory_order_relaxed))
                    break;

                CheckImmediateCommand();
                frameReady = FrameLoop::FrameReady();

                if (!frameReady)
                {
                    framePreparedEvent.Wait();
                }
            }
        }

        if (resetPending.load(std::memory_order_relaxed))
        {
            do
            {
                resetPending = false;
                FrameLoop::RejectFrames();
                DispatchPlatform::ResetBlock();
            } while (resetPending.load());
            resetDoneEvent.Signal();
            frameDoneEvent.Signal();
        }
        else if (frameReady)
        {
            if (DispatchPlatform::ValidateSurface())
                FrameLoop::ProcessFrame();
            else
                FrameLoop::RejectFrames();
            frameDoneEvent.Signal();
        }
    }

    Logger::Info("[RHI] render-thread finished");
}

void InitializeRenderLoop(uint32 frameCount, DAVA::Thread::eThreadPriority priority, int32 bindToProcessor)
{
    DVASSERT(frameCount <= frameSyncObjectsCount);

    renderThreadFrameCount = frameCount;
    FrameLoop::Initialize(frameSyncObjectsCount);

    if (renderThreadFrameCount)
    {
        renderThread = DAVA::Thread::Create(DAVA::Thread::Procedure(&RenderFunc));
        renderThread->SetName("RHI.RENDER_THREAD");
        renderThread->Start();
        if (bindToProcessor != -1)
            renderThread->BindToProcessor(bindToProcessor);
        renderThread->SetPriority(priority);
        renderThredStartedSync.Wait();
    }
    else
    {
        DispatchPlatform::InitContext();
    }
}

void SuspendRender()
{
    DVASSERT(!renderThreadSuspended);
    renderThreadSuspended = true;
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
        DispatchPlatform::FinishRendering();
    }

    Logger::Error("Render  Suspended");
}

void ResumeRender()
{
    DVASSERT(renderThreadSuspended);
    Logger::Error("Render Resumed");
    renderThreadSuspended = false;
    if (renderThreadFrameCount)
    {
        renderThreadSuspendSync.Post();
    }
}

void UninitializeRenderLoop()
{
    if (renderThreadFrameCount)
    {
        renderThreadExitPending = true;
        if (renderThreadSuspended)
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

void SetResetPending()
{
    Logger::Debug("ResetPending");
    if (renderThreadFrameCount == 0)
    {
        FrameLoop::RejectFrames();
        DispatchPlatform::ResetBlock();
    }
    else
    {
        resetPending = true;
        framePreparedEvent.Signal();
        resetDoneEvent.Wait();
    }
}

void IssueImmediateCommand(CommonImpl::ImmediateCommand* command)
{
    DAVA_PROFILER_CPU_SCOPE(DAVA::ProfilerCPUMarkerName::RHI_WAIT_IMMEDIATE_CMDS);

    if (command->forceImmediate || (renderThreadFrameCount == 0))
    {
        DispatchPlatform::ProcessImmediateCommand(command);
    }
    else
    {
        bool scheduled = false;
        bool executed = false;

        // CRAP: busy-wait

        while (!scheduled)
        {
            DAVA::LockGuard<DAVA::Mutex> lock(pendingImmediateCmdSync);
            if (pendingImmediateCmd.load() == nullptr)
            {
                pendingImmediateCmd = command;
                scheduled = true;
            }
        }

        while (!executed)
        {
            if (pendingImmediateCmd.load(std::memory_order_relaxed) == nullptr)
            {
                executed = true;
            }
            if (!executed)
            {
                framePreparedEvent.Signal();
                DAVA::Thread::Yield();
            }
        }
    }
}
void CheckImmediateCommand()
{
    if (pendingImmediateCmd.load(std::memory_order_relaxed) != nullptr)
    {
        CommonImpl::ImmediateCommand* cmd = pendingImmediateCmd.load();
        if (cmd != nullptr)
        {
            DAVA_PROFILER_CPU_SCOPE(DAVA::ProfilerCPUMarkerName::RHI_EXECUTE_IMMEDIATE_CMDS);
            DispatchPlatform::ProcessImmediateCommand(cmd);
            pendingImmediateCmd = nullptr;
        }
    }
}

void ScheduleResourceDeletion(Handle handle, ResourceType resourceType)
{
    scheduledDeleteMutex.Lock();
    scheduledDeleteResources[currFrameSyncId].push_back({ handle, resourceType });
    scheduledDeleteMutex.Unlock();
}

void ProcessScheduledDelete()
{
    DAVA_PROFILER_CPU_SCOPE(DAVA::ProfilerCPUMarkerName::RHI_PROCESS_SCHEDULED_DELETE);

    scheduledDeleteMutex.Lock();
    for (int i = 0; i < frameSyncObjectsCount; i++)
    {
        if (frameSyncObjects[i].IsValid() && SyncObjectSignaled(frameSyncObjects[i]))
        {
            for (std::vector<ScheduledDeleteResource>::iterator it = scheduledDeleteResources[i].begin(), e = scheduledDeleteResources[i].end(); it != e; ++it)
            {
                ScheduledDeleteResource& res = *it;
                switch (res.resourceType)
                {
                case RESOURCE_VERTEX_BUFFER:
                    VertexBuffer::Delete(res.handle);
                    break;
                case RESOURCE_INDEX_BUFFER:
                    IndexBuffer::Delete(res.handle);
                    break;
                case RESOURCE_CONST_BUFFER:
                    ConstBuffer::Delete(res.handle);
                    break;
                case RESOURCE_TEXTURE:
                    Texture::Delete(res.handle);
                    break;
                case RESOURCE_TEXTURE_SET:
                    TextureSet::Delete(res.handle);
                    break;
                case RESOURCE_DEPTHSTENCIL_STATE:
                    DepthStencilState::Delete(res.handle);
                    break;
                case RESOURCE_SAMPLER_STATE:
                    SamplerState::Delete(res.handle);
                    break;
                case RESOURCE_QUERY_BUFFER:
                    QueryBuffer::Delete(res.handle);
                    break;
                case RESOURCE_PIPELINE_STATE:
                    PipelineState::Delete(res.handle);
                    break;
                default:
                    DVASSERT_MSG(false, "Not supported resource scheduled for deletion");
                }
            }
            scheduledDeleteResources[i].clear();
            DeleteSyncObject(frameSyncObjects[i]);
            frameSyncObjects[i] = HSyncObject();
        }
    }
    scheduledDeleteMutex.Unlock();
}

HSyncObject GetCurrentFrameSyncObject()
{
    DAVA::LockGuard<DAVA::Spinlock> lock(scheduledDeleteMutex);
    if (!frameSyncObjects[currFrameSyncId].IsValid())
        frameSyncObjects[currFrameSyncId] = CreateSyncObject();
    return frameSyncObjects[currFrameSyncId];
}
}
}