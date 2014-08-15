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

#include "Render/RenderTarget/RenderTargetDescriptor.h"

namespace DAVA
{

void RenderTargetDescriptor::AddFramebuffer(FramebufferDescriptor framebuffer)
{
    DVASSERT(framebuffer.IsValid());

    RenderTargetDescriptor::FramebufferInfo* curInfo = FindOrCreateFramebufferInfo(framebuffer.GetFramebufferType());

    curInfo->framebuffer = framebuffer;
}

void RenderTargetDescriptor::AddFramebuffer(FramebufferDescriptor framebuffer,
                                            RenderTextureDescriptor texture)
{
    DVASSERT(framebuffer.IsValid());
    DVASSERT(texture.IsValid());

    RenderTargetDescriptor::FramebufferInfo* curInfo = FindOrCreateFramebufferInfo(framebuffer.GetFramebufferType());

    curInfo->framebuffer = framebuffer;
    curInfo->renderTexture = texture;
}

const RenderTargetDescriptor::FramebufferInfo* RenderTargetDescriptor::FindFramebufferInfo(FramebufferDescriptor::FramebufferType framebufferType) const
{
    const RenderTargetDescriptor::FramebufferInfo* curInfo = NULL;

    size_t bufferCount = framebuffers.size();
    for(size_t i = 0; i < bufferCount; ++i)
    {
        if(framebuffers[i].framebuffer.GetFramebufferType() == framebufferType)
        {
            curInfo = &(framebuffers[i]);
            break;
        }
    }

    return curInfo;
}

void RenderTargetDescriptor::RemoveFramebuffer(FramebufferDescriptor::FramebufferType framebufferType)
{
    size_t bufferCount = framebuffers.size();
    for(size_t i = 0; i < bufferCount; ++i)
    {
        if(framebuffers[i].framebuffer.GetFramebufferType() == framebufferType)
        {
            framebuffers[i] = framebuffers[framebuffers.size() - 1];
            framebuffers.pop_back();

            break;
        }
    }
}

void RenderTargetDescriptor::Clear()
{
    framebuffers.clear();
}

uint32 RenderTargetDescriptor::GetFramebufferCount() const
{
    return (uint32)framebuffers.size();
}

const RenderTargetDescriptor::FramebufferInfo& RenderTargetDescriptor::GetFramebufferInfo(uint32 index) const
{
    DVASSERT(index >= 0 && index < framebuffers.size());
    return framebuffers[index];
}

RenderTargetDescriptor::FramebufferInfo* RenderTargetDescriptor::FindOrCreateFramebufferInfo(FramebufferDescriptor::FramebufferType framebufferType)
{
    RenderTargetDescriptor::FramebufferInfo* curInfo = GetFramebufferInfoByType(framebufferType);

    if(NULL == curInfo)
    {
        framebuffers.push_back(RenderTargetDescriptor::FramebufferInfo());
        curInfo = &(framebuffers[framebuffers.size() - 1]);
    }

    return curInfo;
}

bool RenderTargetDescriptor::Validate() const
{
    bool result = true;

    for(uint32 framebufferType = FramebufferDescriptor::FRAMEBUFFER_COLOR0;
        framebufferType < FramebufferDescriptor::FRAMEBUFFER_COUNT;
        ++framebufferType)
    {
        result = IsUnique((FramebufferDescriptor::FramebufferType)framebufferType);

        if(!result)
        {
            break;
        }
    }

    if(result)
    {
        size_t bufferCount = framebuffers.size();
        for(size_t i = 0; i < bufferCount; ++i)
        {
            result = framebuffers[i].framebuffer.IsValid();

            if(!result)
            {
                break;
            }
        }
    }

    return result;
}

bool RenderTargetDescriptor::IsUnique(FramebufferDescriptor::FramebufferType framebufferType) const
{
    bool result = true;

    uint32 typeCount = 0;
    size_t bufferCount = framebuffers.size();
    for(size_t i = 0; i < bufferCount; ++i)
    {
        if(framebuffers[i].framebuffer.GetFramebufferType() == framebufferType)
        {
            ++typeCount;

            if(typeCount > 1)
            {
                result = false;
                break;
            }
        }
    }

    return result;
}

RenderTargetDescriptor::FramebufferInfo* RenderTargetDescriptor::GetFramebufferInfoByType(FramebufferDescriptor::FramebufferType framebufferType)
{
    RenderTargetDescriptor::FramebufferInfo* curInfo = NULL;

    size_t bufferCount = framebuffers.size();
    for(size_t i = 0; i < bufferCount; ++i)
    {
        if(framebuffers[i].framebuffer.GetFramebufferType() == framebufferType)
        {
            curInfo = &(framebuffers[i]);
            break;
        }
    }

    return curInfo;
}

};