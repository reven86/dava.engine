#include "FrameLoop.h"
#include "rhi_Pool.h"
#include "CommonImpl.h"

namespace rhi
{
namespace FrameLoop
{
static uint32 currFrameNumber = 0;
static DAVA::Vector<FrameBase> frames;
static DAVA::Spinlock frameSync;
static bool frameStarted = false;

static void ExecuteFrameCommands()
{
    TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "ExecuteFrameCommands");

    std::vector<RenderPassBase*> pass;
    std::vector<Handle> pass_h;
    currFrameNumber++;

    //sort and test
    frameSync.Lock();
    DVASSERT(frames.size()); //if no frames ready should not call frame loop

    //sort passes
    for (std::vector<Handle>::iterator p = frames.begin()->pass.begin(), p_end = frames.begin()->pass.end(); p != p_end; ++p)
    {
        RenderPassBase* pp = DispatchPlatform::GetRenderPass(*p);
        bool do_add = true;

        for (unsigned i = 0; i != pass.size(); ++i)
        {
            if (pp->priority > pass[i]->priority)
            {
                pass.insert(pass.begin() + i, 1, pp);
                do_add = false;
                break;
            }
        }

        if (do_add)
            pass.push_back(pp);
    }

    pass_h = frames.begin()->pass;

    if (frames.begin()->sync != InvalidHandle)
    {
        SyncObjectBase* sync = DispatchPlatform::GetSyncObject(frames.begin()->sync);

        sync->frame = currFrameNumber;
        sync->is_signaled = false;
        sync->is_used = true;
    }
    frameSync.Unlock();

    //execute CB in passes
    for (std::vector<RenderPassBase *>::iterator p = pass.begin(), p_end = pass.end(); p != p_end; ++p)
    {
        RenderPassBase* pp = *p;

        for (unsigned b = 0; b != pp->cmdBuf.size(); ++b)
        {
            Handle cb_h = pp->cmdBuf[b];

            TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "cb::exec");
            DispatchPlatform::ExecuteCommandBuffer(pp->cmdBuf[b]); // it should also set sync object for command buffer
            TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "cb::exec");

            //platform or not
            DispatchPlatform::FreeCommandBuffer(cb_h);
        }
    }

    frameSync.Lock();
    {
        frames.erase(frames.begin());

        for (std::vector<Handle>::iterator p = pass_h.begin(), p_end = pass_h.end(); p != p_end; ++p)
            DispatchPlatform::FreeRenderPass(*p);
    }
    frameSync.Unlock();

    TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "ExecuteFrameCommands");
}

void RejectFrames()
{
    frameSync.Lock();
    for (std::vector<FrameBase>::iterator f = frames.begin(); f != frames.end();)
    {
        if (f->readyToExecute)
        {
            if (f->sync != InvalidHandle)
            {
                SyncObjectBase* s = DispatchPlatform::GetSyncObject(f->sync);
                s->is_signaled = true;
                s->is_used = true;
            }
            for (std::vector<Handle>::iterator p = f->pass.begin(), p_end = f->pass.end(); p != p_end; ++p)
            {
                RenderPassBase* pp = DispatchPlatform::GetRenderPass(*p);

                for (std::vector<Handle>::iterator c = pp->cmdBuf.begin(), c_end = pp->cmdBuf.end(); c != c_end; ++c)
                {
                    DispatchPlatform::RejectCommandBuffer(*c);
                }
                DispatchPlatform::FreeRenderPass(*p);
            }
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

    if (!CommonDetail::renderContextReady) //no render context - just reject frames and do nothing;
    {
        RejectFrames();
        return;
    }

    bool presentResult = false;
    if (!CommonDetail::resetPending)
    {
        ExecuteFrameCommands();

        TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "PresntBuffer");
        presentResult = DispatchPlatform::PresntBuffer();
        TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "PresntBuffer");
    }

    if (!presentResult)
    {
        RejectFrames();
        DispatchPlatform::ResetBlock();
    }

    DispatchPlatform::UpdateSyncObjects(currFrameNumber);

    TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "ProcessFrame");
}

bool FinishFrame(Handle sync)
{
    size_t frame_cnt = 0;
    frameSync.Lock();
    {
        frame_cnt = frames.size();
        if (frame_cnt)
        {
            frames.back().readyToExecute = true;
            frames.back().sync = sync;
            frameStarted = false;
        }
    }
    FrameLoop::frameSync.Unlock();

    if (!frame_cnt)
    {
        if (sync != InvalidHandle) //frame is empty - still need to sync if required
        {
            SyncObjectBase* syncObject = DispatchPlatform::GetSyncObject(sync);
            syncObject->is_signaled = true;
        }
        return false;
    }

    return true;
}

bool FrameReady()
{
    frameSync.Lock();
    bool res = (FrameLoop::frames.size() && FrameLoop::frames.begin()->readyToExecute);
    frameSync.Unlock();
    return res;
}

uint32 FramesCount()
{
    FrameLoop::frameSync.Lock();
    uint32 frame_cnt = static_cast<uint32>(FrameLoop::frames.size());
    FrameLoop::frameSync.Unlock();
    return frame_cnt;
}
}
}