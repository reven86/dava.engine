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
        RENDER_VERIFY(glDeleteFramebuffers(1, &framebufferId));
    }
}

void RenderTargetOGL::Initialize()
{
    DVASSERT(0 == framebufferId);

    RENDER_VERIFY(glGenFramebuffers(1, &framebufferId));
}

void RenderTargetOGL::BindRenderTarget()
{
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
        SortColorAttachments();

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

    RenderManager::Instance()->ClipPush();
	RenderManager::Instance()->PushDrawMatrix();
	RenderManager::Instance()->PushMappingMatrix();
	RenderManager::Instance()->IdentityDrawMatrix();

    BindRenderTarget();

    ProcessPreRenderActions();
}

void RenderTargetOGL::EndRender()
{
    ProcessPostRenderActions();

    UnbindRenderTarget();

    RenderManager::Instance()->PopDrawMatrix();
	RenderManager::Instance()->PopMappingMatrix();
	RenderManager::Instance()->ClipPop();
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

void RenderTargetOGL::ProcessPreRenderActions()
{
    bool needClearColor = false;
    size_t colorAttachmentCount = colorAttachments.size();
    for(size_t i = 0; i < colorAttachmentCount; ++i)
    {
        if(colorAttachments[i]->GetPreRenderAction() == FramebufferDescriptor::PRE_ACTION_CLEAR)
        {
            needClearColor = true;
            break;
        }
    }

    bool needClearDepth = (depthAttachment != NULL &&
                           depthAttachment->GetPreRenderAction() == FramebufferDescriptor::PRE_ACTION_CLEAR);

    bool needClearStencil = (stencilAttachment != NULL &&
                             stencilAttachment->GetPreRenderAction() == FramebufferDescriptor::PRE_ACTION_CLEAR);

    Rect viewport;
    CalculateViewport(viewport);
    RenderManager::Instance()->SetViewport(viewport, true);
    RenderManager::Instance()->RemoveClip();

    if(needClearColor &&
       needClearDepth &&
       needClearStencil)
    {
        RenderManager::Instance()->Clear(clearColor, clearDepth, clearStencil);
    }
    else
    {
        if(needClearColor)
        {
            RenderManager::Instance()->ClearWithColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        }

        if(needClearDepth)
        {
            RenderManager::Instance()->ClearDepthBuffer(clearDepth);
        }

        if(needClearStencil)
        {
            RenderManager::Instance()->ClearStencilBuffer(clearStencil);
        }
    }
}

void RenderTargetOGL::ProcessPostRenderActions()
{
    uint32 discardFlags = 0;

    size_t colorAttachmentCount = colorAttachments.size();
    for(size_t i = 0; i < colorAttachmentCount; ++i)
    {
        if(colorAttachments[i]->GetPostRenderAction() == FramebufferDescriptor::POST_ACTION_DONTCARE)
        {
            discardFlags = discardFlags | RenderManager::COLOR_ATTACHMENT;
            break;
        }
    }

    if(depthAttachment != NULL &&
       depthAttachment->GetPostRenderAction() == FramebufferDescriptor::POST_ACTION_DONTCARE)
    {
        discardFlags = discardFlags | RenderManager::DEPTH_ATTACHMENT;
    }

    if(stencilAttachment != NULL &&
       stencilAttachment->GetPostRenderAction() == FramebufferDescriptor::POST_ACTION_DONTCARE)
    {
        discardFlags = discardFlags | RenderManager::STENCIL_ATTACHMENT;
    }

    if(discardFlags != 0)
    {
        RenderManager::Instance()->DiscardFramebufferHW(discardFlags);
    }
}

void RenderTargetOGL::CalculateViewport(Rect& viewport)
{
    uint32 maxWidth = 0;
    uint32 maxHeight = 0;

    size_t colorAttachmentCount = colorAttachments.size();
    for(size_t i = 0; i < colorAttachmentCount; ++i)
    {
        FramebufferAttachment* attachment = colorAttachments[i];

        uint32 attachmentWidth = attachment->GetFramebufferWidth();
        uint32 attachmentHeight = attachment->GetFramebufferHeight();

        if(attachmentWidth > maxWidth)
        {
            maxWidth = attachmentWidth;
        }

        if(attachmentHeight > maxHeight)
        {
            maxHeight = attachmentHeight;
        }
    }

    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.dx = (float32)maxWidth;
    viewport.dy = (float32)maxHeight;
}

};