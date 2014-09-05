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
#include "Render/RenderManager.h"

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
impl(bufferId)
{
}

ColorFramebufferAttachmentOGL::ColorFramebufferAttachmentOGL(FramebufferDescriptor::FramebufferType type, Texture* tx) :
ColorFramebufferAttachment(type),
impl(tx)
{
}

ColorFramebufferAttachmentOGL::~ColorFramebufferAttachmentOGL()
{
    impl.DestroyAttachmentData();
}

Texture* ColorFramebufferAttachmentOGL::Lock()
{
    return impl.LockTexture();
}

void ColorFramebufferAttachmentOGL::Unlock(Texture* tx)
{
    impl.UnlockTexture(tx);
}

void ColorFramebufferAttachmentOGL::AttachRenderBuffer()
{
    impl.AttachRenderBuffer(COLOR_ATTACHMENT_ID_MAP[framebufferType], cubeFace, mipLevel);
}

void ColorFramebufferAttachmentOGL::OnActiveMipLevelChanged()
{
    impl.UpdateTextureProperties(COLOR_ATTACHMENT_ID_MAP[framebufferType],
                                 cubeFace,
                                 mipLevel);
}

void ColorFramebufferAttachmentOGL::OnActiveFaceChanged()
{
    impl.UpdateTextureProperties(COLOR_ATTACHMENT_ID_MAP[framebufferType],
                                 cubeFace,
                                 mipLevel);
}

};