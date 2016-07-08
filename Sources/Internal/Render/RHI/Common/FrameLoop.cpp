
#include "rhi_Pool.h"
#include "CommonImpl.h"

namespace rhi
{
namespace FrameLoop
{
static void ProcessFrame()
{
    TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "exec_que_cmds");

    std::vector<RenderPassBase*> pass;
    std::vector<Handle> pass_h;
    unsigned frame_n = 0;

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
    frame_n = frames.begin()->frame_n;

    if (frames.begin()->sync != InvalidHandle)
    {
        SyncObjectBase* sync = DispatchPlatform::GetSyncObject(frames.begin()->sync);

        sync->frame = frame_n;
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

    if (CommonDetail::renderContextReady)
    {
        // do swap-buffers
        TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "gl_end_frame");

#if defined(__DAVAENGINE_WIN32__)
        Trace("rhi-gl.swap-buffers...\n");
        SwapBuffers(_GLES2_WindowDC);
        Trace("rhi-gl.swap-buffers done\n");
#elif defined(__DAVAENGINE_MACOS__)
        macos_gl_end_frame();
#elif defined(__DAVAENGINE_IPHONE__)
        ios_gl_end_frame();
#elif defined(__DAVAENGINE_ANDROID__)

        bool success = android_gl_end_frame();
        if (!success) //'false' mean lost context, need restore resources
        {
            _RejectAllFrames();

            TextureGLES2::ReCreateAll();
            VertexBufferGLES2::ReCreateAll();
            IndexBufferGLES2::ReCreateAll();
        }

#endif

        TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "gl_end_frame");
    }

    TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "exec_que_cmds");
}
}
}