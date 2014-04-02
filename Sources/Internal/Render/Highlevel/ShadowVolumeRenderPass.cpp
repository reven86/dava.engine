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


#include "Render/Highlevel/ShadowVolumeRenderPass.h"
#include "Render/Highlevel/RenderLayer.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/ShadowRect.h"
#include "Render/Highlevel/RenderBatchArray.h"

namespace DAVA
{
    
ShadowVolumeRenderLayer::ShadowVolumeRenderLayer(const FastName & name, uint32 sortingFlags, RenderLayerID id)
    :   RenderLayer(name, sortingFlags, id), shadowRect(NULL)
{
    
	blendMode = ShadowPassBlendMode::MODE_BLEND_ALPHA;
	
}

void ShadowVolumeRenderLayer::CreateShadowRect()
{
    shadowRect = ShadowRect::Create();
    RenderStateData stateData;

    stateData.state =	RenderStateData::STATE_BLEND |
        RenderStateData::STATE_STENCIL_TEST |
        RenderStateData::STATE_COLORMASK_ALL;
    stateData.sourceFactor = BLEND_DST_COLOR;
    stateData.destFactor = BLEND_ZERO;
    stateData.depthFunc = CMP_LEQUAL;
    stateData.cullMode = FACE_BACK;
    stateData.fillMode = FILLMODE_SOLID;
    stateData.stencilFail[0] = stateData.stencilFail[1] = STENCILOP_KEEP;
    stateData.stencilPass[0] = stateData.stencilPass[1] = STENCILOP_KEEP;
    stateData.stencilZFail[0] = stateData.stencilZFail[1] = STENCILOP_KEEP;
    stateData.stencilFunc[0] = stateData.stencilFunc[1] = CMP_NOTEQUAL;
    stateData.stencilMask = 15;
    stateData.stencilRef = 0;

    blendMultiplyState = RenderManager::Instance()->CreateRenderState(stateData);

    stateData.sourceFactor = BLEND_SRC_ALPHA;
    stateData.destFactor = BLEND_ONE_MINUS_SRC_ALPHA;

    blendAlphaState = RenderManager::Instance()->CreateRenderState(stateData);
}

ShadowVolumeRenderLayer::~ShadowVolumeRenderLayer()
{
    SafeRelease(shadowRect);
}

void ShadowVolumeRenderLayer::SetBlendMode(ShadowPassBlendMode::eBlend _blendMode)
{
	blendMode = _blendMode;
}

void ShadowVolumeRenderLayer::Draw(const FastName & ownerRenderPass, Camera * camera, RenderLayerBatchArray * renderLayerBatchArray)
{	
    if (!shadowRect)
    {
        CreateShadowRect();
    }
    if(RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::SHADOWVOLUME_DRAW))
	{
		RenderLayer::Draw(ownerRenderPass, camera, renderLayerBatchArray);			
		RenderManager::Instance()->SetRenderState((ShadowPassBlendMode::MODE_BLEND_ALPHA == blendMode) ? blendAlphaState : blendMultiplyState);									
		shadowRect->Draw();
	}
	
#if 0
    // Draw all layers with their materials
    uint32 size = (uint32)renderLayers.size();
    DVASSERT(size == 1);
    
    RenderLayer * shadowVolumesLayer = renderLayers[0];
    //Vector<LightNode*> & lights = renderSystem->GetLights();
    uint32 renderBatchCount = shadowVolumesLayer->GetRenderBatchCount();
    
    if(RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::SHADOWVOLUME_DRAW)
       && renderBatchCount > 0)
	{
		eBlendMode sFactor = RenderManager::Instance()->GetSrcBlend();
		eBlendMode dFactor = RenderManager::Instance()->GetDestBlend();

		//2nd pass
		RenderManager::Instance()->RemoveState(RenderState::STATE_CULL);
		RenderManager::Instance()->RemoveState(RenderState::STATE_DEPTH_WRITE);
		RenderManager::Instance()->AppendState(RenderState::STATE_BLEND);
		RenderManager::Instance()->SetBlendMode(BLEND_ZERO, BLEND_ONE);
        
		//RenderManager::Instance()->ClearStencilBuffer(0); //moved to per-frame clear
		RenderManager::Instance()->AppendState(RenderState::STATE_STENCIL_TEST);
		
		RenderManager::State()->SetStencilFunc(FACE_FRONT_AND_BACK, CMP_ALWAYS);
		RenderManager::State()->SetStencilRef(1);
		RenderManager::State()->SetStencilMask(0x0000000F);
        
		RenderManager::State()->SetStencilPass(FACE_FRONT, STENCILOP_KEEP);
		RenderManager::State()->SetStencilFail(FACE_FRONT, STENCILOP_KEEP);
		RenderManager::State()->SetStencilZFail(FACE_FRONT, STENCILOP_DECR_WRAP);
        
		RenderManager::State()->SetStencilPass(FACE_BACK, STENCILOP_KEEP);
		RenderManager::State()->SetStencilFail(FACE_BACK, STENCILOP_KEEP);
		RenderManager::State()->SetStencilZFail(FACE_BACK, STENCILOP_INCR_WRAP);
		
		RenderManager::Instance()->FlushState();
    
        shadowVolumesLayer->Draw(camera);
        
		//3rd pass
		RenderManager::Instance()->RemoveState(RenderState::STATE_CULL);
		RenderManager::Instance()->RemoveState(RenderState::STATE_DEPTH_TEST);
		
		RenderManager::State()->SetStencilRef(0);
		RenderManager::State()->SetStencilFunc(FACE_FRONT_AND_BACK, CMP_NOTEQUAL);
		RenderManager::State()->SetStencilPass(FACE_FRONT_AND_BACK, STENCILOP_KEEP);
		RenderManager::State()->SetStencilFail(FACE_FRONT_AND_BACK, STENCILOP_KEEP);
		RenderManager::State()->SetStencilZFail(FACE_FRONT_AND_BACK, STENCILOP_KEEP);

		switch(blendMode)
		{
		case MODE_BLEND_ALPHA:
			RenderManager::Instance()->SetBlendMode(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
			break;
		case MODE_BLEND_MULTIPLY:
			RenderManager::Instance()->SetBlendMode(BLEND_DST_COLOR, BLEND_ZERO);
			break;
		default:
			break;
		}

		shadowRect->Draw();

		RenderManager::Instance()->SetBlendMode(sFactor, dFactor);
	}
#endif
}
    
ShadowRect * ShadowVolumeRenderLayer::GetShadowRect()
{
    if (!shadowRect)
    {
        CreateShadowRect();
    }
    return shadowRect;
}

ShadowPassBlendMode::eBlend ShadowVolumeRenderLayer::GetBlendMode() const
{
	return blendMode;
}

    
};
