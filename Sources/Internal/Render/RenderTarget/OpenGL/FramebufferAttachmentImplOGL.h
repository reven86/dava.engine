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


#ifndef __DAVAENGINE_FRAMEBUFFERATTACHMENTIMPLOGL_H__
#define __DAVAENGINE_FRAMEBUFFERATTACHMENTIMPLOGL_H__

#include "DAVAEngine.h"
#include "Render/Texture.h"


namespace DAVA
{

class FramebufferAttachmentImplOGL
{

    friend class StencilFramebufferAttachmentOGL;

public:

    inline FramebufferAttachmentImplOGL();
    inline FramebufferAttachmentImplOGL(GLuint bufferId);
    inline FramebufferAttachmentImplOGL(Texture* texture);

    inline void DestroyAttachmentData();
    inline void UpdateTextureProperties(GLuint target,
                                        Texture::CubemapFace face,
                                        uint32 mipLevel);
    inline Texture* LockTexture();
    inline void UnlockTexture(Texture* tx);
    inline void AttachRenderBuffer(GLuint target,
                                   Texture::CubemapFace face,
                                   uint32 mipLevel);

private:

    GLuint renderbufferId;
    Texture* resolveTexture;
};

inline FramebufferAttachmentImplOGL::FramebufferAttachmentImplOGL()
{
    renderbufferId = 0;
    resolveTexture = NULL;
}

inline FramebufferAttachmentImplOGL::FramebufferAttachmentImplOGL(GLuint bufferId)
{
    renderbufferId = bufferId;
    resolveTexture = NULL;
}

inline FramebufferAttachmentImplOGL::FramebufferAttachmentImplOGL(Texture* texture)
{
    resolveTexture = SafeRetain(texture);
    renderbufferId = 0;
}

inline void FramebufferAttachmentImplOGL::DestroyAttachmentData()
{
    if(0 != renderbufferId)
    {
        RENDER_VERIFY(glDeleteRenderbuffers(1, &renderbufferId));
    }

    SafeRelease(resolveTexture);
}

inline void FramebufferAttachmentImplOGL::UpdateTextureProperties(GLuint target,
                                                                  Texture::CubemapFace face,
                                                                  uint32 mipLevel)
{
    if(NULL != resolveTexture)
    {
        if(Texture::TEXTURE_2D == resolveTexture->textureType)
        {

            RENDER_VERIFY(glFramebufferTexture2D(GL_FRAMEBUFFER,
                                                 target,
                                                 GL_TEXTURE_2D,
                                                 resolveTexture->id,
                                                 mipLevel));
        }
        else
        {
            RENDER_VERIFY(glFramebufferTexture2D(GL_FRAMEBUFFER,
                                                 target,
                                                 Texture::MapFaceNameToGLName(face),
                                                 resolveTexture->id,
                                                 mipLevel));
        }
    }
}

inline Texture* FramebufferAttachmentImplOGL::LockTexture()
{
    return SafeRetain(resolveTexture);
}

inline void FramebufferAttachmentImplOGL::UnlockTexture(Texture* tx)
{
    DVASSERT(tx == resolveTexture);
    SafeRelease(tx);
}

inline void FramebufferAttachmentImplOGL::AttachRenderBuffer(GLuint target,
                                                             Texture::CubemapFace face,
                                                             uint32 mipLevel)
{
    if(resolveTexture != NULL)
    {
        UpdateTextureProperties(target, face, mipLevel);
    }
    else
    {
        RENDER_VERIFY(glFramebufferRenderbuffer(GL_FRAMEBUFFER, target, GL_RENDERBUFFER, renderbufferId));
    }
}

};

#endif /* defined(__DAVAENGINE_FRAMEBUFFERATTACHMENTIMPLOGL_H__) */
