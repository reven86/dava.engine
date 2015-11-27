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

#ifndef __VisibilityCheckRenderPass_h__
#define __VisibilityCheckRenderPass_h__

#include "Render/Highlevel/RenderPass.h"
#include "Render/RenderBase.h"
#include "Render/Texture.h"

namespace DAVA
{
class VisibilityCheckRenderPass : public RenderPass
{
public:
    VisibilityCheckRenderPass();
    ~VisibilityCheckRenderPass();

    void PreRenderScene(RenderSystem* renderSystem, Camera* camera, Texture* renderTarget);
    void RenderToCubemapFromPoint(RenderSystem* renderSystem, Camera* camera, Texture* renderTarget, const Vector3& point);
    void RenderVisibilityToTexture(RenderSystem* renderSystem, Camera* camera, Texture* cubemap, Texture* renderTarget,
                                   const Vector3& point, const Color& color);

private:
    void SetupCameraToRenderFromPointToFaceIndex(const Vector3& point, uint32 faceIndex);
    void RenderWithCurrentSettings(RenderSystem* renderSystem, Camera* sceneCamera);
    bool ShouldRenderObject(RenderObject*);
    bool ShouldRenderBatch(RenderBatch*);

private:
    ScopedPtr<Camera> camera;
    ScopedPtr<NMaterial> distanceMaterial;
    ScopedPtr<NMaterial> visibilityMaterial;
    ScopedPtr<NMaterial> prerenderMaterial;
    rhi::HDepthStencilState visibilityDepthStencilState;
    rhi::RenderPassConfig renderTargetConfig;
    rhi::RenderPassConfig visibilityConfig;
    rhi::RenderPassConfig prerenderConfig;
};
}

#endif