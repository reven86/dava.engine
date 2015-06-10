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


#include "Render/Material/NMaterial.h"
#include "Render/Material/NMaterialHelper.h"

namespace DAVA
{
//////////////////////////////////////////////////////////////////////////////////////

void NMaterialHelper::EnableStateFlags(const FastName& passName,
                                       NMaterial* target,
                                       uint32 stateFlags)
{
    DVASSERT(target);
    
    RenderStateData newData;
    target->GetRenderState(passName, newData);
    
    newData.state = newData.state | stateFlags;
    
    target->SubclassRenderState(passName, newData);
}

void NMaterialHelper::DisableStateFlags(const FastName& passName,
                                        NMaterial* target,
                                        uint32 stateFlags)
{
    DVASSERT(target);
    
    RenderStateData newData;
    target->GetRenderState(passName, newData);
    
    newData.state = newData.state & ~stateFlags;
    
    target->SubclassRenderState(passName, newData);
}

void NMaterialHelper::SetBlendMode(const FastName& passName,
                                   NMaterial* target,
                                   eBlendMode src,
                                   eBlendMode dst)
{
    DVASSERT(target);
    
    RenderStateData newData;
    target->GetRenderState(passName, newData);
    
    newData.sourceFactor = src;
    newData.destFactor = dst;
    
    target->SubclassRenderState(passName, newData);
}

void NMaterialHelper::SwitchTemplate(NMaterial* material,
                                     const FastName& templateName)
{
    const NMaterialTemplate* matTemplate = NMaterialTemplateCache::Instance()->Get(templateName);
    DVASSERT(matTemplate);
    
    material->SetMaterialTemplate(matTemplate, material->currentQuality);
}

Texture* NMaterialHelper::GetEffectiveTexture(const FastName& textureName, NMaterial* mat)
{
    DVASSERT(mat);
    
    NMaterial::TextureBucket* bucket = mat->GetEffectiveTextureBucket(textureName);
    return (bucket) ? bucket->GetTexture() : NULL;
}

bool NMaterialHelper::IsAlphatest(const FastName& passName, NMaterial* mat)
{
    DVASSERT(mat);
    DVASSERT(mat->baseTechnique);
    
    bool result = false;
    if(mat->baseTechnique)
    {
        result = (mat->baseTechnique->GetLayersSet().count(DAVA::LAYER_ALPHA_TEST_LAYER) > 0);
    }
    
    return result;
}

bool NMaterialHelper::IsOpaque(const FastName& passName, NMaterial* mat)
{
    DVASSERT(mat);
    DVASSERT(mat->baseTechnique);
    
    bool result = false;
    if(mat->baseTechnique)
    {
        result = (mat->baseTechnique->GetLayersSet().count(DAVA::LAYER_OPAQUE) > 0);
    }
    
    return result;
}

bool NMaterialHelper::IsAlphablend(const FastName& passName, NMaterial* mat)
{
    DVASSERT(mat);
    
    bool result = false;
    
    if(mat->instancePasses.count(passName) > 0)
    {
        RenderStateData currentData = mat->GetRenderState(passName);
        
        result = ((currentData.state & RenderStateData::STATE_BLEND) != 0);
    }
    
    return result;
}

bool NMaterialHelper::IsTwoSided(const FastName& passName, NMaterial* mat)
{
    DVASSERT(mat);
    
    bool result = false;
    const RenderStateData& currentData = mat->GetRenderState(passName);
    
    result = ((currentData.state & RenderStateData::STATE_CULL) == 0);
    
    return result;
}

void NMaterialHelper::SetFillMode(const FastName& passName,
                                  NMaterial* mat,
                                  eFillMode fillMode)
{
    DVASSERT(mat);
    
    RenderStateData newData;
    mat->GetRenderState(passName, newData);
    
    newData.fillMode = fillMode;
    
    mat->SubclassRenderState(passName, newData);
}

eFillMode NMaterialHelper::GetFillMode(const FastName& passName, NMaterial* mat)
{
    DVASSERT(mat);
    
    const RenderStateData& currentData = mat->GetRenderState(passName);
    return currentData.fillMode;
}

void NMaterialHelper::SetStencilFunc(const FastName& passName, NMaterial* target, eCmpFunc stencilFunc0, eCmpFunc stencilFunc1, int32 stencilRef, uint32 stencilMask)
{
    DVASSERT(target);

    RenderStateData newData;
    target->GetRenderState(passName, newData);

    newData.stencilFunc[0] = stencilFunc0;
    newData.stencilFunc[1] = stencilFunc1;
    newData.stencilRef = stencilRef;
    newData.stencilMask = stencilMask;

    target->SubclassRenderState(passName, newData);
}

void NMaterialHelper::SetStencilOp(const FastName& passName, NMaterial* target, eStencilOp passFront, eStencilOp passBack, eStencilOp failFront, eStencilOp failBack, eStencilOp zFailFront, eStencilOp zFailBack)
{
    DVASSERT(target);

    RenderStateData newData;
    target->GetRenderState(passName, newData);

    newData.stencilPass[0] = passFront;
    newData.stencilPass[1] = passBack;
    newData.stencilFail[0] = failFront;
    newData.stencilFail[1] = failBack;
    newData.stencilZFail[0] = zFailFront;
    newData.stencilZFail[1] = zFailBack;

    target->SubclassRenderState(passName, newData);
}

};