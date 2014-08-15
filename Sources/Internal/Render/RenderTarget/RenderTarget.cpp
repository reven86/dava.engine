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


#include "Render/RenderTarget/RenderTarget.h"

namespace DAVA
{

RenderTarget::RenderTarget() :  depthAttachment(NULL),
                                stencilAttachment(NULL)
{
}

RenderTarget::~RenderTarget()
{
    size_t colorAttachmentCount = colorAttachments.size();
    for(size_t i = 0; i < colorAttachmentCount; ++i)
    {
        SafeRelease(colorAttachments[i]);
    }

    SafeRelease(depthAttachment);
    SafeRelease(stencilAttachment);
}

ColorFramebufferAttachment* RenderTarget::GetColorAttachment(uint32 index)
{
    DVASSERT(index >= 0 && index < colorAttachments.size());

    return colorAttachments[index];
}

uint32 RenderTarget::GetColorAttachmentCount()
{
    return colorAttachments.size();
}

DepthFramebufferAttachment* RenderTarget::GetDepthAttachment()
{
    return depthAttachment;
}

StencilFramebufferAttachment* RenderTarget::GetStencilAttachment()
{
    return stencilAttachment;
}

void RenderTarget::AddUniqueColorAttachment(ColorFramebufferAttachment* attachment)
{
    DVASSERT(attachment);

    FramebufferDescriptor::FramebufferType framebufferType = attachment->GetFramebufferType();

    bool needAddAttachment = true;
    size_t colorBufferCount = colorAttachments.size();
    for(size_t i = 0; i < colorBufferCount; ++i)
    {
        ColorFramebufferAttachment* currentAttachment = colorAttachments[i];

        if(currentAttachment == attachment)
        {
            needAddAttachment = false;
            break;
        }

        if(currentAttachment->GetFramebufferType() == framebufferType)
        {
            SafeRelease(colorAttachments[i]);
            colorAttachments[i] = SafeRetain(attachment);

            needAddAttachment = false;
            break;
        }
    }

    if(needAddAttachment)
    {
        colorAttachments.push_back(SafeRetain(attachment));
    }
}

void RenderTarget::SetClearColor(const Color& color)
{
    clearColor = color;
}

const Color& RenderTarget::GetClearColor() const
{
    return clearColor;
}

void RenderTarget::SetClearDepth(const float32 depthValue)
{
    clearDepth = depthValue;
}

float32 RenderTarget::GetClearDepth() const
{
    return clearDepth;
}

void RenderTarget::SetClearStencil(int32 stencilValue)
{
    clearStencil = stencilValue;
}

uint32 RenderTarget::GetClearStencil() const
{
    return clearStencil;
}

};
