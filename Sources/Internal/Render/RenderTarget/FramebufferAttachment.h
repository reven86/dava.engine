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


#ifndef __DAVAENGINE_FRAMEBUFFERATTACHMENT_H__
#define __DAVAENGINE_FRAMEBUFFERATTACHMENT_H__

#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"
#include "Base/BaseMath.h"
#include "Base/BaseObject.h"
#include "Base/FastName.h"

#include "Render/Texture.h"
#include "Render/RenderTarget/FramebufferDescriptor.h"

namespace DAVA
{

class FramebufferAttachment : public BaseObject
{
protected:

    virtual ~FramebufferAttachment();

    virtual void OnActiveMipLevelChanged();
    virtual void OnActiveFaceChanged();

public:

    FramebufferAttachment(FramebufferDescriptor::FramebufferType type);

    void SetActiveMipLevel(uint32 activeLevel);
    uint32 GetActiveMipLevel() const;

    void SetActiveFace(Texture::CubemapFace activeFace);
    Texture::CubemapFace GetActiveFace() const;

    void SetPreRenderAction(FramebufferDescriptor::PreRenderAction action);
    FramebufferDescriptor::PreRenderAction GetPreRenderAction() const;

    void SetPostRenderAction(FramebufferDescriptor::PostRenderAction action);
    FramebufferDescriptor::PostRenderAction GetPostRenderAction() const;

    virtual Texture* Lock();
    virtual void Unlock(Texture* tx);

    FramebufferDescriptor::FramebufferType GetFramebufferType() const;

protected:

    uint32 mipLevel;
    Texture::CubemapFace cubeFace;
    FramebufferDescriptor::FramebufferType framebufferType;
    FramebufferDescriptor::PreRenderAction preRenderAction;
    FramebufferDescriptor::PostRenderAction postRenderAction;
};

};

#endif /* defined(__DAVAENGINE_FRAMEBUFFERATTACHMENT_H__) */
