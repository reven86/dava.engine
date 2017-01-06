#include "FrameLoop.h"
#include "rhi_Pool.h"
#include "../rhi_Public.h"
#include "rhi_Private.h"
#include "rhi_CommonImpl.h"
#include "Debug/ProfilerCPU.h"
#include "Debug/ProfilerMarkerNames.h"
#include "Concurrency/Thread.h"

namespace rhi
{
namespace FrameLoop
{
struct ScheduledDeleteResource
{
    Handle handle;
    ResourceType resourceType;
};

static uint32 currentFrameNumber = 0;
static DAVA::Array<CommonImpl::Frame, FRAME_POOL_SIZE> frames;
static uint32 frameToBuild = 0;
static uint32 frameToExecute = 0;
static DAVA::Spinlock frameSync;

static uint32 currFrameSyncId = 0;
static DAVA::Array<HSyncObject, FRAME_POOL_SIZE> frameSyncObjects;
static DAVA::Array<std::vector<ScheduledDeleteResource>, FRAME_POOL_SIZE> scheduledDeleteResources;
static DAVA::Spinlock scheduledDeleteMutex;
static void ProcessScheduledDelete();

void RejectFrames()
{
    DAVA::LockGuard<DAVA::Spinlock> lock(frameSync);
    while (frameToExecute < frameToBuild)
    {
        DispatchPlatform::RejectFrame(frames[frameToExecute % FRAME_POOL_SIZE]);
        frames[frameToExecute % FRAME_POOL_SIZE].Reset();
        frameToExecute++;
    }
    if (frameToExecute >= FRAME_POOL_SIZE)
    {
        frameToBuild -= FRAME_POOL_SIZE;
        frameToExecute -= FRAME_POOL_SIZE;
    }
    if (frames[frameToBuild].pass.size())
    {
        frames[frameToBuild].discarded = true;
    }
}

void ProcessFrame()
{
    bool presentResult = true;
    if (NeedRestoreResources())
    {
        RejectFrames();
    }
    else
    {
        bool frameRejected = false;
        if (frames[frameToExecute].discarded)
        {
            DispatchPlatform::RejectFrame(frames[frameToExecute]);
            frameRejected = true;
        }
        else
        {
            DAVA_PROFILER_CPU_SCOPE_WITH_FRAME_INDEX(DAVA::ProfilerCPUMarkerName::RHI_EXECUTE_FRAME, currentFrameNumber);

            frames[frameToExecute].frameNumber = currentFrameNumber++;
            DispatchPlatform::ExecuteFrame(frames[frameToExecute]);
        }
        frames[frameToExecute].Reset();
        frameSync.Lock();
        frameToExecute++;
        if (frameToExecute >= FRAME_POOL_SIZE)
        {
            frameToBuild -= FRAME_POOL_SIZE;
            frameToExecute -= FRAME_POOL_SIZE;
        }
        frameSync.Unlock();

        if (!frameRejected)
        {
            DAVA_PROFILER_CPU_SCOPE(DAVA::ProfilerCPUMarkerName::RHI_DEVICE_PRESENT);
            presentResult = DispatchPlatform::PresentBuffer();
        }
    }

    ProcessScheduledDelete();

    if (!presentResult)
    {
        RejectFrames();
        DispatchPlatform::ResetBlock();
    }
}

bool FinishFrame()
{
    scheduledDeleteMutex.Lock();
    if (scheduledDeleteResources[currFrameSyncId].size() && !frameSyncObjects[currFrameSyncId].IsValid())
        frameSyncObjects[currFrameSyncId] = CreateSyncObject();
    Handle sync = frameSyncObjects[currFrameSyncId];
    currFrameSyncId = (currFrameSyncId + 1) % FRAME_POOL_SIZE;
    DVASSERT(scheduledDeleteResources[currFrameSyncId].empty()); //we are not going to mix new resources for deletion with existing once still waiting
    DVASSERT(!frameSyncObjects[currFrameSyncId].IsValid());
    scheduledDeleteMutex.Unlock();

    DispatchPlatform::FinishFrame();
    bool frameValid = false;
    frameSync.Lock();
    uint32 frameSlot = frameToBuild % FRAME_POOL_SIZE;
    if (frames[frameSlot].pass.size() != 0)
    {
        frames[frameSlot].readyToExecute = true;
        frames[frameSlot].sync = sync;
        frameToBuild++;
        frameValid = true;
    }
    frameSync.Unlock();
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
    frames[frameToBuild % FRAME_POOL_SIZE].pass.push_back(pass);
}

void SetFramePerfQueries(Handle startQuery, Handle endQuery)
{
    DAVA::LockGuard<DAVA::Spinlock> lock(frameSync);
    CommonImpl::Frame& frame = frames[frameToBuild % FRAME_POOL_SIZE];
    frame.perfQueryStart = startQuery;
    frame.perfQueryEnd = endQuery;
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
    for (int i = 0; i < FRAME_POOL_SIZE; i++)
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
                case RESOURCE_PERFQUERY:
                    PerfQuery::Delete(res.handle);
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
