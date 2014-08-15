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


#ifndef __DAVAENGINE_RENDERTARGETDESCRIPTOR_H__
#define __DAVAENGINE_RENDERTARGETDESCRIPTOR_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/BaseObject.h"

#include "Render/RenderTarget/RenderTextureDescriptor.h"
#include "Render/RenderTarget/FramebufferDescriptor.h"

namespace DAVA
{

class RenderTargetDescriptor
{
public:

    struct FramebufferInfo
    {
        FramebufferDescriptor framebuffer;
        RenderTextureDescriptor renderTexture;
    };

public:

    RenderTargetDescriptor();

    void SetClearColor(Color color);
    const Color& GetClearColor() const;

    void SetClearDepth(float32 value);
    float32 GetClearDepth() const;

    void SetClearStencil(uint32 value);
    uint32 GetClearStencil() const;

    void AddFramebuffer(FramebufferDescriptor framebuffer);
    void AddFramebuffer(FramebufferDescriptor framebuffer, RenderTextureDescriptor texture);

    const RenderTargetDescriptor::FramebufferInfo* FindFramebufferInfo(FramebufferDescriptor::FramebufferType framebufferType) const;

    void RemoveFramebuffer(FramebufferDescriptor::FramebufferType framebufferType);
    void Clear();

    uint32 GetFramebufferCount() const;
    const RenderTargetDescriptor::FramebufferInfo& GetFramebufferInfo(uint32 index) const;

    bool Validate() const;

protected:

    RenderTargetDescriptor::FramebufferInfo* GetFramebufferInfoByType(FramebufferDescriptor::FramebufferType framebufferType);
    RenderTargetDescriptor::FramebufferInfo* FindOrCreateFramebufferInfo(FramebufferDescriptor::FramebufferType framebufferType);
    bool IsUnique(FramebufferDescriptor::FramebufferType framebufferType) const;

protected:

    Vector<RenderTargetDescriptor::FramebufferInfo> framebuffers;
    Color clearColor;
    float32 clearDepth;
    int32 clearStencil;
};

};

#endif /* defined(__DAVAENGINE_RENDERTARGETDESCRIPTOR_H__) */
