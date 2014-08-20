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


#include "Render/RenderTarget/OpenGL/StencilFramebufferAttachmentOGL.h"
#include "Render/RenderTarget/OpenGL/FramebufferAttachmentHelper.h"

#include "Render/RenderManager.h"

namespace DAVA
{

StencilFramebufferAttachmentOGL::StencilFramebufferAttachmentOGL(GLuint bufferId) :
renderbufferId(bufferId),
resolveTexture(NULL),
depthStencil(NULL)
{
}

StencilFramebufferAttachmentOGL::StencilFramebufferAttachmentOGL(Texture* tx) :
renderbufferId(0),
depthStencil(NULL)
{
    resolveTexture = SafeRetain(tx);
}

StencilFramebufferAttachmentOGL::StencilFramebufferAttachmentOGL(DepthFramebufferAttachmentOGL* sharedDepthStencil) :
renderbufferId(0),
resolveTexture(NULL)
{
    depthStencil = SafeRetain(sharedDepthStencil);
}

StencilFramebufferAttachmentOGL::~StencilFramebufferAttachmentOGL()
{
    if(renderbufferId != 0)
    {
        RENDER_VERIFY(glDeleteRenderbuffers(1, &renderbufferId));
    }

    if(resolveTexture != NULL)
    {
        SafeRelease(resolveTexture);
    }

    if(depthStencil != NULL)
    {
        SafeRelease(depthStencil);
    }
}

void StencilFramebufferAttachmentOGL::AttachRenderBuffer()
{
    if(depthStencil != NULL)
    {
        RENDER_VERIFY(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthStencil->renderbufferId));
    }
    else if(renderbufferId != 0)
    {
        RENDER_VERIFY(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbufferId));
    }
    else if(resolveTexture != NULL)
    {
        FramebufferAttachmentHelper::UpdateTextureAttachmentProperties(GL_STENCIL_ATTACHMENT,
                                                                       resolveTexture,
                                                                       cubeFace,
                                                                       mipLevel);
    }
}

Texture* StencilFramebufferAttachmentOGL::Lock()
{
    return SafeRetain(resolveTexture);
}

void StencilFramebufferAttachmentOGL::Unlock(Texture* tx)
{
    SafeRelease(tx);
}

void StencilFramebufferAttachmentOGL::OnActiveMipLevelChanged()
{
    if(resolveTexture != NULL)
    {
        FramebufferAttachmentHelper::UpdateTextureAttachmentProperties(GL_STENCIL_ATTACHMENT,
                                                                       resolveTexture,
                                                                       cubeFace,
                                                                       mipLevel);
    }

}

void StencilFramebufferAttachmentOGL::OnActiveFaceChanged()
{
    if(resolveTexture != NULL)
    {
        FramebufferAttachmentHelper::UpdateTextureAttachmentProperties(GL_STENCIL_ATTACHMENT,
                                                                       resolveTexture,
                                                                       cubeFace,
                                                                       mipLevel);
    }
}

};