/*==================================================================================
 Copyright (c) 2008, binaryzebra
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of the binaryzebra nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 =====================================================================================*/


#include "Render/RenderTarget/OpenGL/RenderTargetOGL.h"

#include "Render/RenderManager.h"

namespace DAVA
{

RenderTargetOGL::RenderTargetOGL() :    framebufferId(0),
                                        prevFramebufferId(0),
                                        renderBuffersAttached(false)
{
}

RenderTargetOGL::~RenderTargetOGL()
{
    UnbindRenderTarget();

    if(framebufferId != 0)
    {
        glDeleteFramebuffers(1, &framebufferId);
    }
}

void RenderTargetOGL::Initialize()
{
    DVASSERT(0 == framebufferId);

    glGenFramebuffers(1, &framebufferId);
}

void RenderTargetOGL::BindRenderTarget()
{
    DVASSERT(0 == prevFramebufferId);

    prevFramebufferId = RenderManager::Instance()->HWglGetLastFBO();

    RenderManager::Instance()->HWglBindFBO(framebufferId);
}

void RenderTargetOGL::UnbindRenderTarget()
{
    RenderManager::Instance()->HWglBindFBO(prevFramebufferId);
    prevFramebufferId = 0;
}

bool RenderTargetOGL::CheckRenderTargetCompleteness()
{
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE)
	{
		Logger::Error("[RenderTargetOGL::CheckRenderTargetCompleteness] glCheckFramebufferStatus: %d", status);
	}

    return (GL_FRAMEBUFFER_COMPLETE == status);
}

void RenderTargetOGL::AttachRenderBuffers()
{
    if(false == renderBuffersAttached)
    {
        size_t colorBufferCount = colorAttachments.size();
        for(size_t i = 0; i < colorBufferCount; ++i)
        {
            ColorFramebufferAttachmentOGL* attachment = (ColorFramebufferAttachmentOGL*)colorAttachments[i];
            attachment->AttachRenderBuffer();
        }

        if(depthAttachment != NULL)
        {
            DepthFramebufferAttachmentOGL* attachment = (DepthFramebufferAttachmentOGL*)depthAttachment;
            attachment->AttachRenderBuffer();
        }

        if(stencilAttachment != NULL)
        {
            StencilFramebufferAttachmentOGL* attachment = (StencilFramebufferAttachmentOGL*)stencilAttachment;
            attachment->AttachRenderBuffer();
        }

        renderBuffersAttached = CheckRenderTargetCompleteness();
    }
}

void RenderTargetOGL::BeginRender()
{
    DVASSERT(renderBuffersAttached);

    BindRenderTarget();
}

void RenderTargetOGL::EndRender()
{
    UnbindRenderTarget();
}

void RenderTargetOGL::AddColorAttachment(ColorFramebufferAttachmentOGL* attachment)
{
    AddUniqueColorAttachment(attachment);
}

void RenderTargetOGL::SetDepthAttachment(DepthFramebufferAttachmentOGL* attachment)
{
    if(attachment != depthAttachment)
    {
        SafeRelease(depthAttachment);
        depthAttachment = SafeRetain(attachment);
    }
}

void RenderTargetOGL::SetStencilAttachment(StencilFramebufferAttachmentOGL* attachment)
{
    if(attachment != stencilAttachment)
    {
        SafeRelease(stencilAttachment);
        stencilAttachment = SafeRetain(attachment);
    }
}

};