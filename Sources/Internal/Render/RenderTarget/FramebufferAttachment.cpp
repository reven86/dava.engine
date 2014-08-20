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


#include "Render/RenderTarget/FramebufferAttachment.h"

namespace DAVA
{

FramebufferAttachment::FramebufferAttachment(FramebufferDescriptor::FramebufferType type) :
    mipLevel(0),
    cubeFace(Texture::CUBE_FACE_POSITIVE_X),
    framebufferType(type),
    preRenderAction(FramebufferDescriptor::PRE_ACTION_DONTCARE),
    postRenderAction(FramebufferDescriptor::POST_ACTION_DONTCARE)
{
}

FramebufferAttachment::~FramebufferAttachment()
{
}

FramebufferDescriptor::FramebufferType FramebufferAttachment::GetFramebufferType() const
{
    return framebufferType;
}

void FramebufferAttachment::SetActiveMipLevel(uint32 activeLevel)
{
    if(activeLevel != mipLevel)
    {
        mipLevel = activeLevel;
        OnActiveMipLevelChanged();
    }
}

uint32 FramebufferAttachment::GetActiveMipLevel() const
{
    return mipLevel;
}

void FramebufferAttachment::SetActiveFace(Texture::CubemapFace activeFace)
{
    if(activeFace != cubeFace)
    {
        cubeFace = activeFace;
        OnActiveFaceChanged();
    }
}

Texture::CubemapFace FramebufferAttachment::GetActiveFace() const
{
    return cubeFace;
}

Texture* FramebufferAttachment::Lock()
{
    return NULL;
}

void FramebufferAttachment::Unlock(Texture* tx)
{
}

void FramebufferAttachment::OnActiveMipLevelChanged()
{
}

void FramebufferAttachment::OnActiveFaceChanged()
{
}

void FramebufferAttachment::SetPreRenderAction(FramebufferDescriptor::PreRenderAction action)
{
    preRenderAction = action;
}

FramebufferDescriptor::PreRenderAction FramebufferAttachment::GetPreRenderAction() const
{
    return preRenderAction;
}

void FramebufferAttachment::SetPostRenderAction(FramebufferDescriptor::PostRenderAction action)
{
    postRenderAction = action;
}

FramebufferDescriptor::PostRenderAction FramebufferAttachment::GetPostRenderAction() const
{
    return postRenderAction;
}

void FramebufferAttachment::SetFramebufferFormat(FramebufferDescriptor::FramebufferFormat format)
{
    framebufferFormat = format;
}

FramebufferDescriptor::FramebufferFormat FramebufferAttachment::GetFramebufferFormat() const
{
    return framebufferFormat;
}

void FramebufferAttachment::SetFramebufferWidth(uint32 width)
{
    framebufferWidth = width;
}

uint32 FramebufferAttachment::GetFramebufferWidth() const
{
    return framebufferWidth;
}

void FramebufferAttachment::SetFramebufferHeight(uint32 height)
{
    framebufferHeight = height;
}

uint32 FramebufferAttachment::GetFramebufferHeight() const
{
    return framebufferHeight;
}

};