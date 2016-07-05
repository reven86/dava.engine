
#include "rhi_Pool.h"
#include "CommonImpl.h"

namespace rhi
{
namespace FrameLoop
{
static void ProcessFrame()
{
    TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "exec_que_cmds");
    std::vector<RenderPassGLES2_t*> pass;
    std::vector<Handle> pass_h;
    unsigned frame_n = 0;
    bool do_exit = false;

    //sort and test
    frameSync.Lock();
    if (_GLES2_Frame.size())
    {
        for (std::vector<Handle>::iterator p = _GLES2_Frame.begin()->pass.begin(), p_end = _GLES2_Frame.begin()->pass.end(); p != p_end; ++p)
        {
            RenderPassGLES2_t* pp = RenderPassPoolGLES2::Get(*p);
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

        pass_h = _GLES2_Frame.begin()->pass;
        frame_n = _GLES2_Frame.begin()->number;
    }
    else
    {
        do_exit = true;
    }
    if (_GLES2_Frame.size() && (_GLES2_Frame.begin()->sync != InvalidHandle))
    {
        SyncObjectGLES2_t* sync = SyncObjectPoolGLES2::Get(_GLES2_Frame.begin()->sync);

        sync->frame = frame_n;
        sync->is_signaled = false;
        sync->is_used = true;
    }
    _GLES2_FrameSync.Unlock();

    if (do_exit)
        return;

    Trace("\n\n-------------------------------\nexecuting frame %u\n", frame_n);
    for (std::vector<RenderPassGLES2_t *>::iterator p = pass.begin(), p_end = pass.end(); p != p_end; ++p)
    {
        RenderPassGLES2_t* pp = *p;

        for (unsigned b = 0; b != pp->cmdBuf.size(); ++b)
        {
            Handle cb_h = pp->cmdBuf[b];
            CommandBufferGLES2_t* cb = CommandBufferPoolGLES2::Get(cb_h);

            TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "cb::exec");
            cb->Execute();
            TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "cb::exec");

            if (cb->sync != InvalidHandle)
            {
                SyncObjectGLES2_t* sync = SyncObjectPoolGLES2::Get(cb->sync);

                sync->frame = frame_n;
                sync->is_signaled = false;
                sync->is_used = true;
            }

            CommandBufferPoolGLES2::Free(cb_h);
        }
    }

    _GLES2_FrameSync.Lock();
    {
        Trace("\n\n-------------------------------\nframe %u executed(submitted to GPU)\n", frame_n);
        _GLES2_Frame.erase(_GLES2_Frame.begin());

        for (std::vector<Handle>::iterator p = pass_h.begin(), p_end = pass_h.end(); p != p_end; ++p)
            RenderPassPoolGLES2::Free(*p);
    }
    _GLES2_FrameSync.Unlock();

    if (_GLES2_Context)
    {
        // take screenshot, if needed

        _GLES2_ScreenshotCallbackSync.Lock();
        if (_GLES2_PendingScreenshotCallback)
        {
            const uint32 stride = 4 * _GLES2_DefaultFrameBuffer_Width;
            uint8* rgba = new uint8[stride * _GLES2_DefaultFrameBuffer_Height];

            GLCommand cmd[] =
            {
              { GLCommand::BIND_FRAMEBUFFER, { GL_FRAMEBUFFER, _GLES2_Default_FrameBuffer } },
              { GLCommand::PIXEL_STORE_I, { GL_PACK_ALIGNMENT, 1 } },
              { GLCommand::READ_PIXELS, { 0, 0, uint64(_GLES2_DefaultFrameBuffer_Width), uint64(_GLES2_DefaultFrameBuffer_Height), GL_RGBA, GL_UNSIGNED_BYTE, uint64(rgba) } },
              { GLCommand::BIND_FRAMEBUFFER, { GL_FRAMEBUFFER, _GLES2_Binded_FrameBuffer } },
            };

            _ExecGL(cmd, countof(cmd));
            for (int y = 0; y < _GLES2_DefaultFrameBuffer_Height / 2; ++y)
            {
                uint8* line1 = rgba + y * stride;
                uint8* line2 = rgba + (_GLES2_DefaultFrameBuffer_Height - y - 1) * stride;
                uint8 tmp[5 * 1024 * 4];

                DVASSERT(stride <= sizeof(tmp));
                memcpy(tmp, line1, stride);
                memcpy(line1, line2, stride);
                memcpy(line2, tmp, stride);
            }
            (*_GLES2_PendingScreenshotCallback)(_GLES2_DefaultFrameBuffer_Width, _GLES2_DefaultFrameBuffer_Height, rgba);
            delete[] rgba;
            _GLES2_PendingScreenshotCallback = nullptr;
        }
        _GLES2_ScreenshotCallbackSync.Unlock();

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

    // update sync-objects

    _GLES2_SyncObjectsSync.Lock();
    for (SyncObjectPoolGLES2::Iterator s = SyncObjectPoolGLES2::Begin(), s_end = SyncObjectPoolGLES2::End(); s != s_end; ++s)
    {
        if (s->is_used && (frame_n - s->frame >= 2))
            s->is_signaled = true;
    }
    _GLES2_SyncObjectsSync.Unlock();

    TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "exec_que_cmds");
}
}
}