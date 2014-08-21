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


#ifndef __DAVAENGINE_RENDERTARGET_H__
#define __DAVAENGINE_RENDERTARGET_H__

#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"
#include "Base/BaseMath.h"
#include "Base/BaseObject.h"
#include "Base/FastName.h"
#include "Render/RenderResource.h"

#include "Render/RenderTarget/FramebufferDescriptor.h"
#include "Render/RenderTarget/ColorFramebufferAttachment.h"
#include "Render/RenderTarget/DepthFramebufferAttachment.h"
#include "Render/RenderTarget/StencilFramebufferAttachment.h"

namespace DAVA
{

/**
\brief

Usage example (the following sample will produce yellow .png file with 1024x1024 dimensions):

 RenderTargetDescriptor rtDesc;
 RenderTargetFactory::Instance()->ConstructGenericTargetDescription(RenderTargetFactory::ATTACHMENT_COLOR_TEXTURE,
 1024,
 1024,
 rtDesc);

 rtDesc.SetClearColor(Color(1.0f, 1.0f, 0.0f, 1.0f));

 RenderTarget* renderTarget = RenderTargetFactory::Instance()->CreateRenderTarget(rtDesc);
 RenderDataReader* renderDataReader = RenderTargetFactory::Instance()->GetRenderDataReader();

 DVASSERT(renderTarget);

 renderTarget->BeginRender();

 //TODO: add some render code here

 renderTarget->EndRender();

 Texture* tx = renderTarget->GetColorAttachment()->Lock();
 
 //you may use texture directly for texturing
 //for example material->SetTexture(NMaterial::TEXTURE_DETAIL, tx);

 Image* textureData = renderDataReader->ReadTextureData(tx);

 ImageSystem::Instance()->Save("~doc:/test_image.png", textureData);

 SafeRelease(textureData);
 SafeRelease(renderDataReader);

 renderTarget->GetColorAttachment()->Unlock(tx);

 SafeRelease(renderTarget);

*/

class RenderTarget : public BaseObject
{

protected:

    ~RenderTarget();

public:

    RenderTarget();

    virtual void BeginRender() = 0;
    virtual void EndRender() = 0;

    ColorFramebufferAttachment* GetColorAttachment(uint32 index = 0);
    uint32 GetColorAttachmentCount();

    DepthFramebufferAttachment* GetDepthAttachment();
    StencilFramebufferAttachment* GetStencilAttachment();

    void SetClearColor(const Color& color);
    const Color& GetClearColor() const;

    void SetClearDepth(const float32 depthValue);
    float32 GetClearDepth() const;

    void SetClearStencil(int32 stencilValue);
    uint32 GetClearStencil() const;

protected:

    void AddUniqueColorAttachment(ColorFramebufferAttachment* attachment);
    void SortColorAttachments();

protected:

    Vector<ColorFramebufferAttachment*> colorAttachments;
    DepthFramebufferAttachment* depthAttachment;
    StencilFramebufferAttachment* stencilAttachment;

    Color clearColor;
    float32 clearDepth;
    int32 clearStencil;
};


};

#endif /* defined(__DAVAENGINE_RENDERTARGET_H__) */
