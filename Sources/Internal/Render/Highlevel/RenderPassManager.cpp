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


#include "Render/Highlevel/RenderPassManager.h"
#include "Render/Highlevel/RenderFastNames.h"
#include "Render/Highlevel/RenderBatchArray.h"
#include "Render/Highlevel/RenderPass.h"
#include "Render/Highlevel/ShadowVolumeRenderPass.h"
#include "Render/Highlevel/RenderLayerManager.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Render/Highlevel/StaticOcclusionRenderPass.h"

namespace DAVA
{

    
void RenderPassManager::InsertPass(RenderPass * renderPass)
{
    array[renderPass->GetRenderPassID()] = renderPass;
    map[renderPass->GetName()] = renderPass;
}

void RenderPassManager::Release()
{
    size_t size = array.size();
    for (size_t i = 0; i < size; ++i)
        SafeDelete(array[i]);
    array.clear();
    map.clear();
}

RenderPassManager::RenderPassManager()
    : array(RENDER_PASS_ID_COUNT)
    , map(NextPowerOf2(RENDER_PASS_ID_COUNT))
{
    const RenderLayerManager * renderLayerManager = RenderLayerManager::Instance();

    RenderPass * forwardPass = new RenderPass(PASS_FORWARD, RENDER_PASS_FORWARD_ID);
    InsertPass(forwardPass);
    
    forwardPass->AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_OPAQUE), LAST_LAYER);
	forwardPass->AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_AFTER_OPAQUE), LAST_LAYER);
	forwardPass->AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_ALPHA_TEST_LAYER), LAST_LAYER);
    forwardPass->AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_TRANSLUCENT), LAST_LAYER);
	forwardPass->AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_AFTER_TRANSLUCENT), LAST_LAYER);
	forwardPass->AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_SHADOW_VOLUME), LAST_LAYER);
    
    /*ShadowVolumeRenderPass * shadowVolumePass = new ShadowVolumeRenderPass(renderSystem, PASS_SHADOW_VOLUME, RENDER_PASS_SHADOW_VOLUME_ID);
    InsertPass(shadowVolumePass);
    shadowVolumePass->AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_SHADOW_VOLUME), LAST_LAYER);*/
    
    //StaticOcclusionRenderPass * staticOcclusionRenderPass = new StaticOcclusionRenderPass(renderSystem, PASS_STATIC_OCCLUSION, RENDER_PASS_STATIC_OCCLUSION_ID);
    //InsertPass(staticOcclusionRenderPass);
    
}

RenderPassManager::~RenderPassManager()
{
    Release();
}

};