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


#include "Render/RenderTarget/OpenGL/ColorFramebufferAttachmentOGL.h"
#include "Render/RenderTarget/OpenGL/FramebufferAttachmentHelper.h"

namespace DAVA
{

static GLenum COLOR_ATTACHMENT_ID_MAP[] =
{
    0, //padding

    GL_COLOR_ATTACHMENT0,
    GL_COLOR_ATTACHMENT1,
    GL_COLOR_ATTACHMENT2,
    GL_COLOR_ATTACHMENT3
};

ColorFramebufferAttachmentOGL::ColorFramebufferAttachmentOGL(FramebufferDescriptor::FramebufferType type,
                                                             GLuint bufferId) :
ColorFramebufferAttachment(type),
renderbufferId(bufferId),
resolveTexture(NULL)
{
}

ColorFramebufferAttachmentOGL::ColorFramebufferAttachmentOGL(FramebufferDescriptor::FramebufferType type, Texture* tx) :
ColorFramebufferAttachment(type),
renderbufferId(0)
{
    resolveTexture = SafeRetain(tx);
}

ColorFramebufferAttachmentOGL::~ColorFramebufferAttachmentOGL()
{
    if(renderbufferId != 0)
    {
        glDeleteRenderbuffers(1, &renderbufferId);
    }

    SafeRelease(resolveTexture);
}

Texture* ColorFramebufferAttachmentOGL::Lock()
{
    //VI: TODO: think about using glReadPixels
    //in case when Lock() is called on a renderbuffer without a resolve texture.

    return SafeRetain(resolveTexture);
}

void ColorFramebufferAttachmentOGL::Unlock(Texture* tx)
{
    SafeRelease(tx);
}

void ColorFramebufferAttachmentOGL::AttachRenderBuffer()
{
    if(resolveTexture != NULL)
    {
        FramebufferAttachmentHelper::UpdateTextureAttachmentProperties(COLOR_ATTACHMENT_ID_MAP[framebufferType],
                                                                       resolveTexture,
                                                                       cubeFace,
                                                                       mipLevel);
    }
    else
    {
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, COLOR_ATTACHMENT_ID_MAP[framebufferType], GL_RENDERBUFFER, renderbufferId);
    }
}

void ColorFramebufferAttachmentOGL::OnActiveMipLevelChanged()
{
    if(resolveTexture != NULL)
    {
        FramebufferAttachmentHelper::UpdateTextureAttachmentProperties(COLOR_ATTACHMENT_ID_MAP[framebufferType],
                                                                       resolveTexture,
                                                                       cubeFace,
                                                                       mipLevel);
    }
}

void ColorFramebufferAttachmentOGL::OnActiveFaceChanged()
{
    if(resolveTexture != NULL)
    {
        FramebufferAttachmentHelper::UpdateTextureAttachmentProperties(COLOR_ATTACHMENT_ID_MAP[framebufferType],
                                                                       resolveTexture,
                                                                       cubeFace,
                                                                       mipLevel);
    }
}

};