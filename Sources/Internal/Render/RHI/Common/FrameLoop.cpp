
#include "rhi_Pool.h"
#include "CommonImpl.h"

namespace rhi
{
namespace FrameLoop
{
static uint32 currFrameNumber = 0;

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
}
}