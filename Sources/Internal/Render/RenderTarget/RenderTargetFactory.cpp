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

void RenderTargetFactory::ConstructGenericTargetDescription(uint32 flags,
                                       uint32 width,
                                       uint32 height,
                                       RenderTargetDescriptor& outDesc)
{
    FramebufferDescriptor::PreRenderAction colorPreRenderAction = FramebufferDescriptor::PRE_ACTION_DONTCARE;
    FramebufferDescriptor::PostRenderAction colorPostRenderAction = FramebufferDescriptor::POST_ACTION_DONTCARE;

    if(flags & RenderTargetFactory::ATTACHMENT_COLOR)
    {
        colorPreRenderAction = FramebufferDescriptor::PRE_ACTION_CLEAR;
        colorPostRenderAction = FramebufferDescriptor::POST_ACTION_STORE;
    }
    else if(flags & RenderTargetFactory::ATTACHMENT_COLOR_TEXTURE)
    {
        colorPreRenderAction = FramebufferDescriptor::PRE_ACTION_CLEAR;
        colorPostRenderAction = FramebufferDescriptor::POST_ACTION_RESOLVE;
    }

    ConstructGenericTargetDescription(flags,
                                      width,
                                      height,
                                      colorPreRenderAction,
                                      colorPostRenderAction,
                                      outDesc);
}

void RenderTargetFactory::ConstructGenericTargetDescription(uint32 flags,
                                                            uint32 width,
                                                            uint32 height,
                                                            FramebufferDescriptor::PreRenderAction colorPreRenderAction,
                                                            FramebufferDescriptor::PostRenderAction colorPostRenderAction,
                                                            RenderTargetDescriptor& outDesc)
{
    FramebufferDescriptor::PreRenderAction depthPreRenderAction = FramebufferDescriptor::PRE_ACTION_DONTCARE;
    FramebufferDescriptor::PostRenderAction depthPostRenderAction = FramebufferDescriptor::POST_ACTION_DONTCARE;
    FramebufferDescriptor::PreRenderAction stencilPreRenderAction = FramebufferDescriptor::PRE_ACTION_DONTCARE;
    FramebufferDescriptor::PostRenderAction stencilPostRenderAction = FramebufferDescriptor::POST_ACTION_DONTCARE;

    if(flags & RenderTargetFactory::ATTACHMENT_DEPTH)
    {
        depthPreRenderAction = FramebufferDescriptor::PRE_ACTION_CLEAR;
        depthPostRenderAction = FramebufferDescriptor::POST_ACTION_DONTCARE;
    }
    else if(flags & RenderTargetFactory::ATTACHMENT_DEPTH_TEXTURE)
    {
        depthPreRenderAction = FramebufferDescriptor::PRE_ACTION_CLEAR;
        depthPostRenderAction = FramebufferDescriptor::POST_ACTION_RESOLVE;
    }

    if(flags & RenderTargetFactory::ATTACHMENT_STENCIL)
    {
        stencilPreRenderAction = FramebufferDescriptor::PRE_ACTION_CLEAR;
        stencilPostRenderAction = FramebufferDescriptor::POST_ACTION_DONTCARE;
    }
    else if(flags & RenderTargetFactory::ATTACHMENT_STENCIL_TEXTURE)
    {
        stencilPreRenderAction = FramebufferDescriptor::PRE_ACTION_CLEAR;
        stencilPostRenderAction = FramebufferDescriptor::POST_ACTION_RESOLVE;
    }

    ConstructGenericTargetDescription(flags,
                                      width,
                                      height,
                                      colorPreRenderAction,
                                      colorPostRenderAction,
                                      depthPreRenderAction,
                                      depthPostRenderAction,
                                      stencilPreRenderAction,
                                      stencilPostRenderAction,
                                      outDesc);
}

void RenderTargetFactory::ConstructGenericTargetDescription(uint32 flags,
                                                            uint32 width,
                                                            uint32 height,
                                                            FramebufferDescriptor::PreRenderAction colorPreRenderAction,
                                                            FramebufferDescriptor::PostRenderAction colorPostRenderAction,
                                                            FramebufferDescriptor::PreRenderAction depthPreRenderAction,
                                                            FramebufferDescriptor::PostRenderAction depthPostRenderAction,
                                                            FramebufferDescriptor::PreRenderAction stencilPreRenderAction,
                                                            FramebufferDescriptor::PostRenderAction stencilPostRenderAction,
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
                                               colorPreRenderAction,
                                               colorPostRenderAction,
                                               width,
                                               height);

        outDesc.AddFramebuffer(colorFramebuffer);
    }
    else if(flags & RenderTargetFactory::ATTACHMENT_COLOR_TEXTURE)
    {
        FramebufferDescriptor colorFramebuffer(FramebufferDescriptor::FRAMEBUFFER_COLOR0,
                                               FramebufferDescriptor::FORMAT_RGBA8888,
                                               colorPreRenderAction,
                                               colorPostRenderAction,
                                               width,
                                               height);

        outDesc.AddFramebuffer(colorFramebuffer, textureDescriptor);
    }

    if(flags & RenderTargetFactory::ATTACHMENT_DEPTH)
    {
        FramebufferDescriptor depthFramebuffer(FramebufferDescriptor::FRAMEBUFFER_DEPTH,
                                               FramebufferDescriptor::FORMAT_DEPTH24,
                                               depthPreRenderAction,
                                               depthPostRenderAction,
                                               width,
                                               height);

        outDesc.AddFramebuffer(depthFramebuffer);
    }
    else if(flags & RenderTargetFactory::ATTACHMENT_DEPTH_TEXTURE)
    {
        FramebufferDescriptor depthFramebuffer(FramebufferDescriptor::FRAMEBUFFER_DEPTH,
                                               FramebufferDescriptor::FORMAT_DEPTH24,
                                               depthPreRenderAction,
                                               depthPostRenderAction,
                                               width,
                                               height);

        outDesc.AddFramebuffer(depthFramebuffer, textureDescriptor);
    }

    if(flags & RenderTargetFactory::ATTACHMENT_STENCIL)
    {
        FramebufferDescriptor stencilFramebuffer(FramebufferDescriptor::FRAMEBUFFER_STENCIL,
                                                 FramebufferDescriptor::FORMAT_STENCIL8,
                                                 stencilPreRenderAction,
                                                 stencilPostRenderAction,
                                                 width,
                                                 height);

        outDesc.AddFramebuffer(stencilFramebuffer);
    }
    else if(flags & RenderTargetFactory::ATTACHMENT_STENCIL_TEXTURE)
    {
        FramebufferDescriptor stencilFramebuffer(FramebufferDescriptor::FRAMEBUFFER_STENCIL,
                                                 FramebufferDescriptor::FORMAT_STENCIL8,
                                                 stencilPreRenderAction,
                                                 stencilPostRenderAction,
                                                 width,
                                                 height);

        outDesc.AddFramebuffer(stencilFramebuffer, textureDescriptor);
    }

    outDesc.SetClearColor(Color(0.0f, 0.0f, 0.0f, 0.0f));
    outDesc.SetClearDepth(0.0f);
    outDesc.SetClearStencil(0);
    
    DVASSERT(outDesc.Validate());
}

RenderTarget* RenderTargetFactory::CreateRenderTarget(uint32 flags,
                                 uint32 width,
                                 uint32 height)
{
    RenderTargetDescriptor rtDesc;
    ConstructGenericTargetDescription(flags, width, height, rtDesc);

    return CreateRenderTarget(rtDesc);
}

RenderTarget* RenderTargetFactory::CreateRenderTarget(uint32 flags,
                                 uint32 width,
                                 uint32 height,
                                 FramebufferDescriptor::PreRenderAction colorPreRenderAction,
                                 FramebufferDescriptor::PostRenderAction colorPostRenderAction)
{
    RenderTargetDescriptor rtDesc;
    ConstructGenericTargetDescription(flags, width, height, colorPreRenderAction, colorPostRenderAction, rtDesc);

    return CreateRenderTarget(rtDesc);

}

RenderTarget* RenderTargetFactory::CreateRenderTarget(uint32 flags,
                                 uint32 width,
                                 uint32 height,
                                 FramebufferDescriptor::PreRenderAction colorPreRenderAction,
                                 FramebufferDescriptor::PostRenderAction colorPostRenderAction,
                                 FramebufferDescriptor::PreRenderAction depthPreRenderAction,
                                 FramebufferDescriptor::PostRenderAction depthPostRenderAction,
                                 FramebufferDescriptor::PreRenderAction stencilPreRenderAction,
                                 FramebufferDescriptor::PostRenderAction stencilPostRenderAction)
{
    RenderTargetDescriptor rtDesc;
    ConstructGenericTargetDescription(flags,
                                      width,
                                      height,
                                      colorPreRenderAction,
                                      colorPostRenderAction,
                                      depthPreRenderAction,
                                      depthPostRenderAction,
                                      stencilPreRenderAction,
                                      stencilPostRenderAction,
                                      rtDesc);

    return CreateRenderTarget(rtDesc);
}


RenderTarget* RenderTargetFactory::CreateRenderTarget(GenericAttachmentFlags colorAttachmentType,
                                                      GenericAttachmentFlags depthAttachmentType,
                                                      GenericAttachmentFlags stencilAttachmentType,
                                                      uint32 width,
                                                      uint32 height,
                                                      FramebufferDescriptor::FramebufferFormat colorFormat,
                                                      FramebufferDescriptor::FramebufferFormat depthFormat,
                                                      FramebufferDescriptor::FramebufferFormat stencilFormat,
                                                      FramebufferDescriptor::PreRenderAction colorPreRenderAction,
                                                      FramebufferDescriptor::PostRenderAction colorPostRenderAction,
                                                      FramebufferDescriptor::PreRenderAction depthPreRenderAction,
                                                      FramebufferDescriptor::PostRenderAction depthPostRenderAction,
                                                      FramebufferDescriptor::PreRenderAction stencilPreRenderAction,
                                                      FramebufferDescriptor::PostRenderAction stencilPostRenderAction)
{
    RenderTargetDescriptor rtDesc;
    ConstructGenericTargetDescription(colorAttachmentType,
                                      depthAttachmentType,
                                      stencilAttachmentType,
                                      width,
                                      height,
                                      colorFormat,
                                      depthFormat,
                                      stencilFormat,
                                      colorPreRenderAction,
                                      colorPostRenderAction,
                                      depthPreRenderAction,
                                      depthPostRenderAction,
                                      stencilPreRenderAction,
                                      stencilPostRenderAction,
                                      rtDesc);

    return CreateRenderTarget(rtDesc);
}


void RenderTargetFactory::ConstructGenericTargetDescription(GenericAttachmentFlags colorAttachmentType,
                                                            GenericAttachmentFlags depthAttachmentType,
                                                            GenericAttachmentFlags stencilAttachmentType,
                                                            uint32 width,
                                                            uint32 height,
                                                            FramebufferDescriptor::FramebufferFormat colorFormat,
                                                            FramebufferDescriptor::FramebufferFormat depthFormat,
                                                            FramebufferDescriptor::FramebufferFormat stencilFormat,
                                                            FramebufferDescriptor::PreRenderAction colorPreRenderAction,
                                                            FramebufferDescriptor::PostRenderAction colorPostRenderAction,
                                                            FramebufferDescriptor::PreRenderAction depthPreRenderAction,
                                                            FramebufferDescriptor::PostRenderAction depthPostRenderAction,
                                                            FramebufferDescriptor::PreRenderAction stencilPreRenderAction,
                                                            FramebufferDescriptor::PostRenderAction stencilPostRenderAction,
                                                            RenderTargetDescriptor& outDesc)
{
    RenderTextureDescriptor textureDesc(Texture::TEXTURE_2D,
                                        Texture::WRAP_CLAMP_TO_EDGE,
                                        Texture::WRAP_CLAMP_TO_EDGE,
                                        Texture::FILTER_LINEAR_MIPMAP_LINEAR,
                                        Texture::FILTER_LINEAR_MIPMAP_LINEAR);

    FramebufferDescriptor colorDesc;
    colorDesc.SetFramebufferType(FramebufferDescriptor::FRAMEBUFFER_COLOR0);
    colorDesc.SetFramebufferFormat(colorFormat);
    colorDesc.SetFramebufferHeight(height);
    colorDesc.SetFramebufferWidth(width);
    colorDesc.SetPreRenderAction(colorPreRenderAction);
    colorDesc.SetPostRenderAction(colorPostRenderAction);

    FramebufferDescriptor depthDesc;
    depthDesc.SetFramebufferType(FramebufferDescriptor::FRAMEBUFFER_DEPTH);
    depthDesc.SetFramebufferFormat(depthFormat);
    depthDesc.SetFramebufferHeight(height);
    depthDesc.SetFramebufferWidth(width);
    depthDesc.SetPreRenderAction(depthPreRenderAction);
    depthDesc.SetPostRenderAction(depthPostRenderAction);

    FramebufferDescriptor stencilDesc;
    stencilDesc.SetFramebufferType(FramebufferDescriptor::FRAMEBUFFER_STENCIL);
    stencilDesc.SetFramebufferFormat(stencilFormat);
    stencilDesc.SetFramebufferHeight(height);
    stencilDesc.SetFramebufferWidth(width);
    stencilDesc.SetPreRenderAction(stencilPreRenderAction);
    stencilDesc.SetPostRenderAction(stencilPostRenderAction);

    outDesc.SetClearColor(Color(0.0f, 0.0f, 0.0f, 0.0f));
    outDesc.SetClearDepth(1.0f);
    outDesc.SetClearStencil(0);

    if(RenderTargetFactory::ATTACHMENT_COLOR == colorAttachmentType)
    {
        outDesc.AddFramebuffer(colorDesc);
    }
    else
    {
        outDesc.AddFramebuffer(colorDesc, textureDesc);
    }

    if(RenderTargetFactory::ATTACHMENT_DEPTH == depthAttachmentType)
    {
        outDesc.AddFramebuffer(depthDesc);
    }
    else
    {
        outDesc.AddFramebuffer(depthDesc, textureDesc);
    }

    if(RenderTargetFactory::ATTACHMENT_STENCIL == stencilAttachmentType)
    {
        outDesc.AddFramebuffer(stencilDesc);
    }
    else
    {
        outDesc.AddFramebuffer(stencilDesc, textureDesc);
    }

}


};