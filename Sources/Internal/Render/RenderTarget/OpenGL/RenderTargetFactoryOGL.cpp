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

#include "Render/RenderTarget/OpenGL/RenderTargetFactoryOGL.h"
#include "Render/RenderTarget/OpenGL/RenderTargetOGL.h"
#include "Render/RenderTarget/OpenGL/ColorFramebufferAttachmentOGL.h"
#include "Render/RenderTarget/OpenGL/DepthFramebufferAttachmentOGL.h"
#include "Render/RenderTarget/OpenGL/StencilFramebufferAttachmentOGL.h"
#include "Render/RenderTarget/OpenGL/RenderDataReaderOGL.h"
#include "Render/RenderManager.h"

namespace DAVA
{

static PixelFormat FRAMEBUFFER_TO_PIXEL_FORMAT_MAP[] =
{
    FORMAT_INVALID,

    FORMAT_RGBA8888,
    FORMAT_A8,
    FORMAT_A16,
    FORMAT_RGBA16161616,
    FORMAT_RGB565,
    FORMAT_RGBA4444,

    FORMAT_A16,
    FORMAT_RGBA8888,
    FORMAT_RGBA8888,

    FORMAT_A8,

    FORMAT_RGBA8888
};

static GLuint FRAMEBUFFER_TO_GL_FORMAT_MAP[] =
{
    FORMAT_INVALID,

    GL_RGBA8,
    GL_ALPHA8,
    GL_ALPHA16,
    GL_RGBA16,
    GL_RGB5,
    GL_RGBA4,

    GL_ALPHA16,
    GL_RGB8,
    GL_RGBA8,
    
    GL_ALPHA8,
};

RenderTargetFactoryOGL::RenderTargetFactoryOGL()
{
    renderDataReader = new RenderDataReaderOGL();
}

RenderTargetFactoryOGL::~RenderTargetFactoryOGL()
{
    SafeRelease(renderDataReader);
}

RenderTarget* RenderTargetFactoryOGL::CreateRenderTarget(const RenderTargetDescriptor& rtDesc)
{
    RenderTargetOGL* renderTarget = NULL;
    bool result = rtDesc.Validate();

    if(result)
    {
        renderTarget = new RenderTargetOGL();
        renderTarget->Initialize();

        renderTarget->SetClearColor(rtDesc.GetClearColor());
        renderTarget->SetClearDepth(rtDesc.GetClearDepth());
        renderTarget->SetClearStencil(rtDesc.GetClearStencil());

        InitColorRenderTargets(renderTarget, rtDesc);

        GLuint depthFormat = 0;
        GLuint stencilFormat = 0;
        bool result = MapDepthStencilFormatToInternalFormat(rtDesc, depthFormat, stencilFormat);

        DVASSERT(result);

        if(result &&
           (depthFormat != 0 &&
           stencilFormat != 0))
        {
            if(depthFormat == stencilFormat)
            {
                InitDepthStencilRenderTarget(renderTarget, rtDesc, depthFormat);
            }
            else
            {
                InitDepthRenderTarget(renderTarget, rtDesc, depthFormat);
                InitStencilRenderTarget(renderTarget, rtDesc, stencilFormat);
            }
        }

        bool renderTargetOK = ((renderTarget->GetColorAttachmentCount() > 0) ||
                               (renderTarget->GetDepthAttachment() != NULL) ||
                               (renderTarget->GetStencilAttachment() != NULL));

        DVASSERT(renderTargetOK);

        if(renderTargetOK)
        {
            renderTarget->BindRenderTarget();

            renderTarget->AttachRenderBuffers();

            renderTarget->UnbindRenderTarget();
        }
        else
        {
            Logger::Error("[RenderTargetFactoryOGL::CreateRenderTarget] Failed to initialize render target!");

            SafeRelease(renderTarget);
        }
    }
    else
    {
        Logger::Error("[RenderTargetFactoryOGL::CreateRenderTarget] Invalid render target configuration!");
    }

    return renderTarget;
}

void RenderTargetFactoryOGL::InitColorRenderTargets(RenderTargetOGL* renderTarget,
                                                    const RenderTargetDescriptor& rtDesc)
{
    for(uint32 i = FramebufferDescriptor::FRAMEBUFFER_COLOR0;
        i <= FramebufferDescriptor::FRAMEBUFFER_COLOR3;
        ++i)
    {
        const RenderTargetDescriptor::FramebufferInfo* framebufferDesc = rtDesc.FindFramebufferInfo((FramebufferDescriptor::FramebufferType)i);

        if(framebufferDesc != NULL)
        {
            GLuint colorInternalFormat = 0;
            bool result = MapColorFormatToInternalFormat(framebufferDesc->framebuffer, colorInternalFormat);

            DVASSERT(result);

            if(result)
            {
                ColorFramebufferAttachmentOGL* attachment = CreateFramebufferAttachment<ColorFramebufferAttachmentOGL>(framebufferDesc, colorInternalFormat);

                renderTarget->AddColorAttachment(attachment);

                SafeRelease(attachment);
            }
        }
    }
}

void RenderTargetFactoryOGL::InitDepthRenderTarget(RenderTargetOGL* renderTarget,
                                                   const RenderTargetDescriptor& rtDesc,
                                                   GLuint depthFormat)
{
    const RenderTargetDescriptor::FramebufferInfo* framebufferDesc = rtDesc.FindFramebufferInfo(FramebufferDescriptor::FRAMEBUFFER_DEPTH);

    if(framebufferDesc != NULL)
    {
        DepthFramebufferAttachmentOGL* attachment = CreateFramebufferAttachment<DepthFramebufferAttachmentOGL>(framebufferDesc, depthFormat);

        renderTarget->SetDepthAttachment(attachment);

        SafeRelease(attachment);
    }
}

void RenderTargetFactoryOGL::InitStencilRenderTarget(RenderTargetOGL* renderTarget,
                                                     const RenderTargetDescriptor& rtDesc,
                                                     GLuint stencilFormat)
{
    const RenderTargetDescriptor::FramebufferInfo* framebufferDesc = rtDesc.FindFramebufferInfo(FramebufferDescriptor::FRAMEBUFFER_STENCIL);

    if(framebufferDesc != NULL)
    {
        StencilFramebufferAttachmentOGL* attachment = CreateFramebufferAttachment<StencilFramebufferAttachmentOGL>(framebufferDesc, stencilFormat);

        renderTarget->SetStencilAttachment(attachment);

        SafeRelease(attachment);
    }

}

void RenderTargetFactoryOGL::InitDepthStencilRenderTarget(RenderTargetOGL* renderTarget,
                                                          const RenderTargetDescriptor& rtDesc,
                                                          GLuint depthStencilFormat)
{
    const RenderTargetDescriptor::FramebufferInfo* depthDesc = rtDesc.FindFramebufferInfo(FramebufferDescriptor::FRAMEBUFFER_DEPTH);
    const RenderTargetDescriptor::FramebufferInfo* stencilDesc = rtDesc.FindFramebufferInfo(FramebufferDescriptor::FRAMEBUFFER_STENCIL);

    if(depthDesc != NULL)
    {
        DepthFramebufferAttachmentOGL* depthAttachment = CreateFramebufferAttachment<DepthFramebufferAttachmentOGL>(depthDesc, depthStencilFormat);

        StencilFramebufferAttachmentOGL* stencilAttachment = new StencilFramebufferAttachmentOGL(depthAttachment);
        stencilAttachment->SetPreRenderAction(stencilDesc->framebuffer.GetPreRenderAction());
        stencilAttachment->SetPostRenderAction(stencilDesc->framebuffer.GetPostRenderAction());
        stencilAttachment->SetFramebufferFormat(stencilDesc->framebuffer.GetFramebufferFormat());
        stencilAttachment->SetFramebufferWidth(stencilDesc->framebuffer.GetFramebufferWidth());
        stencilAttachment->SetFramebufferHeight(stencilDesc->framebuffer.GetFramebufferHeight());

        renderTarget->SetDepthAttachment(depthAttachment);
        renderTarget->SetStencilAttachment(stencilAttachment);

        SafeRelease(depthAttachment);
        SafeRelease(stencilAttachment);
    }
}

Texture* RenderTargetFactoryOGL::CreateResolveTexture(const FramebufferDescriptor& framebufferDesc,
                                                      const RenderTextureDescriptor& textureDesc) const
{
    Texture* tx = NULL;

    DVASSERT(textureDesc.IsValid());

    if(textureDesc.IsValid())
    {
        FramebufferDescriptor::FramebufferFormat framebufferFormat = framebufferDesc.GetFramebufferFormat();
        PixelFormat pixelFormat = MapFramebufferFormatToOGLFormat(framebufferFormat);

        tx = Texture::CreateFromData(pixelFormat,
                                     NULL,
                                    framebufferDesc.GetFramebufferWidth(),
                                    framebufferDesc.GetFramebufferHeight(),
                                    false);
    }

    return tx;
}

template<class T> T* RenderTargetFactoryOGL::CreateFramebufferAttachment(const RenderTargetDescriptor::FramebufferInfo* framebufferDesc,
                                                                         GLuint requestedInternalFormat)
{
    DVASSERT(framebufferDesc);

    T* attachment = NULL;

    if(framebufferDesc->renderTexture.IsValid())
    {
        Texture* tx = CreateResolveTexture(framebufferDesc->framebuffer, framebufferDesc->renderTexture);

        DVASSERT(tx);

        attachment = (T*)NewAttachment(framebufferDesc, tx);

        SafeRelease(tx);
    }
    else
    {
        GLuint bufferId = 0;
        RENDER_VERIFY(glGenRenderbuffers(1, &bufferId));

        DVASSERT(bufferId != 0);

        RENDER_VERIFY(glBindRenderbuffer(GL_RENDERBUFFER, bufferId));

        RENDER_VERIFY(glRenderbufferStorage(GL_RENDERBUFFER,
                              requestedInternalFormat,
                              framebufferDesc->framebuffer.GetFramebufferWidth(),
                              framebufferDesc->framebuffer.GetFramebufferHeight()));

        attachment = (T*)NewAttachment(framebufferDesc, bufferId);
    }

    DVASSERT(attachment);

    attachment->SetPreRenderAction(framebufferDesc->framebuffer.GetPreRenderAction());
    attachment->SetPostRenderAction(framebufferDesc->framebuffer.GetPostRenderAction());
    attachment->SetFramebufferFormat(framebufferDesc->framebuffer.GetFramebufferFormat());
    attachment->SetFramebufferWidth(framebufferDesc->framebuffer.GetFramebufferWidth());
    attachment->SetFramebufferHeight(framebufferDesc->framebuffer.GetFramebufferHeight());

    return attachment;
}

bool RenderTargetFactoryOGL::MapDepthStencilFormatToInternalFormat(const RenderTargetDescriptor& rtDesc,
                                           GLuint& outDepthFormat,
                                           GLuint& outStencilFormat)
{
    const RenderTargetDescriptor::FramebufferInfo* depthDesc = rtDesc.FindFramebufferInfo(FramebufferDescriptor::FRAMEBUFFER_DEPTH);
    const RenderTargetDescriptor::FramebufferInfo* stencilDesc = rtDesc.FindFramebufferInfo(FramebufferDescriptor::FRAMEBUFFER_STENCIL);

    bool depthFormatSelected = (NULL == depthDesc);
    bool stencilFormatSelected = (NULL == stencilDesc);

    if(depthDesc != NULL &&
       stencilDesc != NULL)
    {
        bool combinedDepthStencilSupported = true;

#if defined(__DAVAENGINE_ANDROID__)
        combinedDepthStencilSupported = RenderManager::Instance()->GetCaps().isGlDepth24Stencil8Supported;
#endif

        FramebufferDescriptor::FramebufferFormat depthFormat = depthDesc->framebuffer.GetFramebufferFormat();
        FramebufferDescriptor::FramebufferFormat stencilFormat = stencilDesc->framebuffer.GetFramebufferFormat();

        if(combinedDepthStencilSupported &&
           ((FramebufferDescriptor::FORMAT_DEPTH16 == depthFormat) ||
            (FramebufferDescriptor::FORMAT_DEPTH24 == depthFormat)) &&
           (FramebufferDescriptor::FORMAT_STENCIL8 == stencilFormat))
        {
            outDepthFormat = GL_DEPTH24_STENCIL8;
            outStencilFormat = GL_DEPTH24_STENCIL8;

            depthFormatSelected = true;
            stencilFormatSelected = true;
        }
    }

#if defined(__DAVAENGINE_ANDROID__)
    if(false == depthFormatSelected)
    {
        if((FramebufferDescriptor::FORMAT_DEPTH16 == depthDesc->framebuffer.GetFramebufferFormat()) &&
           RenderManager::Instance()->GetCaps().isGlDepthNvNonLinearSupported)
        {
            outDepthFormat = GL_DEPTH_COMPONENT16_NONLINEAR_NV;
            depthFormatSelected = true;
        }
    }
#endif

    if(false == depthFormatSelected)
    {
        outDepthFormat = FRAMEBUFFER_TO_GL_FORMAT_MAP[depthDesc->framebuffer.GetFramebufferFormat()];
        depthFormatSelected = true;
    }

    if(false == stencilFormatSelected)
    {
        outStencilFormat = FRAMEBUFFER_TO_GL_FORMAT_MAP[stencilDesc->framebuffer.GetFramebufferFormat()];
        stencilFormatSelected = true;
    }

    return (depthFormatSelected && stencilFormatSelected);
}

bool RenderTargetFactoryOGL::MapColorFormatToInternalFormat(const FramebufferDescriptor& framebufferDesc,
                                                            GLuint& outColorFormat)
{
    outColorFormat = FRAMEBUFFER_TO_GL_FORMAT_MAP[framebufferDesc.GetFramebufferFormat()];

    return true;
}

FramebufferAttachment* RenderTargetFactoryOGL::NewAttachment(const RenderTargetDescriptor::FramebufferInfo* framebufferDesc,
                                                            Texture* tx)
{
    FramebufferAttachment* result = NULL;

    FramebufferDescriptor::FramebufferType framebufferType = framebufferDesc->framebuffer.GetFramebufferType();

    if(FramebufferDescriptor::FRAMEBUFFER_COLOR0 == framebufferType ||
       FramebufferDescriptor::FRAMEBUFFER_COLOR1 == framebufferType ||
       FramebufferDescriptor::FRAMEBUFFER_COLOR2 == framebufferType ||
       FramebufferDescriptor::FRAMEBUFFER_COLOR3 == framebufferType)
    {
        result = new ColorFramebufferAttachmentOGL(framebufferType, tx);
    }
    else if(FramebufferDescriptor::FRAMEBUFFER_DEPTH == framebufferType)
    {
        result = new DepthFramebufferAttachmentOGL(tx);
    }
    else if(FramebufferDescriptor::FRAMEBUFFER_STENCIL == framebufferType)
    {
        result = new StencilFramebufferAttachmentOGL(tx);
    }
    else
    {
        DVASSERT(false && "[RenderTargetFactoryOGL::NewAttachment] Unknown framebuffer attachment type requested!");
    }

    return result;
}

FramebufferAttachment* RenderTargetFactoryOGL::NewAttachment(const RenderTargetDescriptor::FramebufferInfo* framebufferDesc,
                                                             GLuint bufferId)
{
    FramebufferAttachment* result = NULL;

    FramebufferDescriptor::FramebufferType framebufferType = framebufferDesc->framebuffer.GetFramebufferType();

    if(FramebufferDescriptor::FRAMEBUFFER_COLOR0 == framebufferType ||
       FramebufferDescriptor::FRAMEBUFFER_COLOR1 == framebufferType ||
       FramebufferDescriptor::FRAMEBUFFER_COLOR2 == framebufferType ||
       FramebufferDescriptor::FRAMEBUFFER_COLOR3 == framebufferType)
    {
        result = new ColorFramebufferAttachmentOGL(framebufferType, bufferId);
    }
    else if(FramebufferDescriptor::FRAMEBUFFER_DEPTH == framebufferType)
    {
        result = new DepthFramebufferAttachmentOGL(bufferId);
    }
    else if(FramebufferDescriptor::FRAMEBUFFER_STENCIL == framebufferType)
    {
        result = new StencilFramebufferAttachmentOGL(bufferId);
    }
    else
    {
        DVASSERT(false && "[RenderTargetFactoryOGL::NewAttachment] Unknown framebuffer attachment type requested!");
    }
    
    return result;
}

PixelFormat RenderTargetFactoryOGL::MapFramebufferFormatToOGLFormat(FramebufferDescriptor::FramebufferFormat format)
{
    return FRAMEBUFFER_TO_PIXEL_FORMAT_MAP[format];
}

RenderDataReader* RenderTargetFactoryOGL::GetRenderDataReader()
{
    return SafeRetain(renderDataReader);
}

};
