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


#include "Render/RenderTarget/RenderTargetFactory.h"

namespace DAVA
{

RenderTargetFactory* RenderTargetFactory::instance = NULL;

RenderTargetFactory::~RenderTargetFactory()
{
}

RenderTargetFactory* RenderTargetFactory::Instance()
{
    DVASSERT(instance);

    return instance;
}

void RenderTargetFactory::InitFactory(RenderTargetFactory* factory)
{
    DVASSERT(NULL == instance);

    ReleaseFactory();

    instance = SafeRetain(factory);
}

void RenderTargetFactory::ReleaseFactory()
{
    SafeRelease(instance);
}

void RenderTargetFactory::ConstructGenericTargetDescription(RenderTargetFactory::GenericAttachmentFlags flags,
                                       uint32 width,
                                       uint32 height,
                                       RenderTargetDescriptor& outDesc)
{
    DVASSERT(((flags & RenderTargetFactory::ATTACHMENT_COLOR) | (flags & RenderTargetFactory::ATTACHMENT_COLOR_TEXTURE)) <= RenderTargetFactory::ATTACHMENT_COLOR_TEXTURE);
    DVASSERT(((flags & RenderTargetFactory::ATTACHMENT_DEPTH) | (flags & RenderTargetFactory::ATTACHMENT_DEPTH_TEXTURE)) <= RenderTargetFactory::ATTACHMENT_DEPTH_TEXTURE);
    DVASSERT(((flags & RenderTargetFactory::ATTACHMENT_STENCIL) | (flags & RenderTargetFactory::ATTACHMENT_STENCIL_TEXTURE)) <= RenderTargetFactory::ATTACHMENT_STENCIL_TEXTURE);


    RenderTextureDescriptor textureDescriptor(Texture::TEXTURE_2D,
                                              Texture::WRAP_CLAMP_TO_EDGE,
                                              Texture::WRAP_CLAMP_TO_EDGE,
                                              Texture::FILTER_LINEAR_MIPMAP_LINEAR,
                                              Texture::FILTER_LINEAR_MIPMAP_LINEAR);

    if(flags & RenderTargetFactory::ATTACHMENT_COLOR)
    {
        FramebufferDescriptor colorFramebuffer(FramebufferDescriptor::FRAMEBUFFER_COLOR0,
                                               FramebufferDescriptor::FORMAT_RGBA8888,
                                               FramebufferDescriptor::PRE_ACTION_CLEAR,
                                               FramebufferDescriptor::POST_ACTION_STORE,
                                               width,
                                               height);

        outDesc.AddFramebuffer(colorFramebuffer);
    }
    else if(flags & RenderTargetFactory::ATTACHMENT_COLOR_TEXTURE)
    {
        FramebufferDescriptor colorFramebuffer(FramebufferDescriptor::FRAMEBUFFER_COLOR0,
                                               FramebufferDescriptor::FORMAT_RGBA8888,
                                               FramebufferDescriptor::PRE_ACTION_CLEAR,
                                               FramebufferDescriptor::POST_ACTION_RESOLVE,
                                               width,
                                               height);

        outDesc.AddFramebuffer(colorFramebuffer, textureDescriptor);
    }

    if(flags & RenderTargetFactory::ATTACHMENT_DEPTH)
    {
        FramebufferDescriptor depthFramebuffer(FramebufferDescriptor::FRAMEBUFFER_DEPTH,
                                               FramebufferDescriptor::FORMAT_DEPTH24,
                                               FramebufferDescriptor::PRE_ACTION_CLEAR,
                                               FramebufferDescriptor::POST_ACTION_DONTCARE,
                                               width,
                                               height);

        outDesc.AddFramebuffer(depthFramebuffer);
    }
    else if(flags & RenderTargetFactory::ATTACHMENT_DEPTH_TEXTURE)
    {
        FramebufferDescriptor depthFramebuffer(FramebufferDescriptor::FRAMEBUFFER_DEPTH,
                                               FramebufferDescriptor::FORMAT_DEPTH24,
                                               FramebufferDescriptor::PRE_ACTION_CLEAR,
                                               FramebufferDescriptor::POST_ACTION_RESOLVE,
                                               width,
                                               height);

        outDesc.AddFramebuffer(depthFramebuffer, textureDescriptor);
    }

    if(flags & RenderTargetFactory::ATTACHMENT_STENCIL)
    {
        FramebufferDescriptor stencilFramebuffer(FramebufferDescriptor::FRAMEBUFFER_STENCIL,
                                               FramebufferDescriptor::FORMAT_STENCIL8,
                                               FramebufferDescriptor::PRE_ACTION_CLEAR,
                                               FramebufferDescriptor::POST_ACTION_DONTCARE,
                                               width,
                                               height);

        outDesc.AddFramebuffer(stencilFramebuffer);
    }
    else if(flags & RenderTargetFactory::ATTACHMENT_STENCIL_TEXTURE)
    {
        FramebufferDescriptor stencilFramebuffer(FramebufferDescriptor::FRAMEBUFFER_STENCIL,
                                                 FramebufferDescriptor::FORMAT_STENCIL8,
                                                 FramebufferDescriptor::PRE_ACTION_CLEAR,
                                                 FramebufferDescriptor::POST_ACTION_RESOLVE,
                                                 width,
                                                 height);

        outDesc.AddFramebuffer(stencilFramebuffer, textureDescriptor);
    }

    outDesc.SetClearColor(Color(0.0f, 0.0f, 0.0f, 0.0f));
    outDesc.SetClearDepth(0.0f);
    outDesc.SetClearStencil(0);

    DVASSERT(outDesc.Validate());
}

};