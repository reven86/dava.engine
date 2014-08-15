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

namespace DAVA
{

RenderTarget* RenderTargetFactoryOGL::CreateRenderTarget(const RenderTargetDescriptor& rtDesc)
{
    RenderTargetOGL* renderTarget = NULL;
    bool result = rtDesc.Validate();

    if(result)
    {
        renderTarget = new RenderTargetOGL();


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

        }
    }
}

void RenderTargetFactoryOGL::InitDepthRenderTarget(RenderTargetOGL* renderTarget,
                                                   const RenderTargetDescriptor& rtDesc)
{

}

void RenderTargetFactoryOGL::InitStencilRenderTarget(RenderTargetOGL* renderTarget,
                                                     const RenderTargetDescriptor& rtDesc)
{

}

void RenderTargetFactoryOGL::InitDepthStencilRenderTarget(RenderTargetOGL* renderTarget,
                                                          const RenderTargetDescriptor& rtDesc)
{

}

};
