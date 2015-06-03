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


#include "Render/Highlevel/RenderLayerManager.h"
#include "Render/Highlevel/RenderFastNames.h"
#include "Render/Highlevel/RenderBatchArray.h"
#include "Render/Highlevel/RenderLayer.h"
#include "Render/Highlevel/ShadowVolumeRenderPass.h"

namespace DAVA
{
    
HashMap<FastName, RenderLayerID> RenderLayerManager::layerIDmap(16);
    
RenderLayerID RenderLayerManager::GetLayerIDByName(const FastName & fastname)
{
    DVASSERT((layerIDmap.find(fastname)!=layerIDmap.end())&&"Unknown material layer");
    return layerIDmap[fastname];
}

uint32 RenderLayerManager::GetLayerIDMaskBySet(const FastNameSet & layers)
{
    uint32 renderLayerIDsBitmask = 0;
    uint32 minLayerID = 100000;
    uint32 maxLayerID = 0;
    renderLayerIDsBitmask = 0;
    
    FastNameSet::iterator layerEnd = layers.end();
    for (FastNameSet::iterator layerIt = layers.begin(); layerIt != layerEnd; ++layerIt)
    {
        RenderLayerID id = GetLayerIDByName(layerIt->first);
        minLayerID = Min(id, minLayerID);
        maxLayerID = Max(id, maxLayerID);
        renderLayerIDsBitmask |= (1 << id);
    }
    return renderLayerIDsBitmask;
}

    
void RenderLayerManager::InsertLayer(RenderLayer * renderLayer)
{
    array[renderLayer->GetRenderLayerID()] = renderLayer;
    map[renderLayer->GetName()] = renderLayer;
    layerIDmap[renderLayer->GetName()] = renderLayer->GetRenderLayerID();
}    

RenderLayerManager::RenderLayerManager()
    : array(RENDER_LAYER_ID_COUNT)
    , map(NextPowerOf2(RENDER_LAYER_ID_COUNT))
{
    RenderLayer * renderLayerOpaque = new RenderLayer(LAYER_OPAQUE,
                                                      RenderLayerBatchArray::SORT_ENABLED | RenderLayerBatchArray::SORT_BY_MATERIAL,
                                                      RENDER_LAYER_OPAQUE_ID);
    InsertLayer(renderLayerOpaque);

    RenderLayer * renderLayerAfterOpaque = new RenderLayer(LAYER_AFTER_OPAQUE,
                                                           RenderLayerBatchArray::SORT_ENABLED | RenderLayerBatchArray::SORT_BY_MATERIAL,
                                                           RENDER_LAYER_AFTER_OPAQUE_ID);
    InsertLayer(renderLayerAfterOpaque);
    
    RenderLayer * renderLayerAlphaTest = new RenderLayer(LAYER_ALPHA_TEST_LAYER,
                                                           RenderLayerBatchArray::SORT_ENABLED | RenderLayerBatchArray::SORT_BY_MATERIAL,
                                                           RENDER_LAYER_ALPHA_TEST_LAYER_ID);
    InsertLayer(renderLayerAlphaTest);

    RenderLayer * renderLayerWater = new RenderLayer(LAYER_WATER, 0, RENDER_LAYER_WATER_ID);
    InsertLayer(renderLayerWater);
    
    RenderLayer * renderLayerTranslucent = new RenderLayer(LAYER_TRANSLUCENT,
                                                         RenderLayerBatchArray::SORT_ENABLED | RenderLayerBatchArray::SORT_BY_DISTANCE_BACK_TO_FRONT,
                                                         RENDER_LAYER_TRANSLUCENT_ID);
    InsertLayer(renderLayerTranslucent);
    
    RenderLayer * renderLayerAfterTranslucent = new RenderLayer(LAYER_AFTER_TRANSLUCENT,
                                                           RenderLayerBatchArray::SORT_ENABLED | RenderLayerBatchArray::SORT_BY_MATERIAL,
                                                           RENDER_LAYER_AFTER_TRANSLUCENT_ID);
    InsertLayer(renderLayerAfterTranslucent);

    RenderLayer * renderLayerShadowVolume = new ShadowVolumeRenderLayer(LAYER_SHADOW_VOLUME,
                                                            0,
                                                            RENDER_LAYER_SHADOW_VOLUME_ID);
    InsertLayer(renderLayerShadowVolume);
    
    RenderLayer * renderLayerVegetation = new RenderLayer(LAYER_VEGETATION,
                                                          0,
                                                          RENDER_LAYER_VEGETATION_ID);
    InsertLayer(renderLayerVegetation);

    RenderLayer * renderLayerDebugDraw = new RenderLayer(LAYER_DEBUG_DRAW, 0, RENDER_LAYER_DEBUG_DRAW_ID);
    InsertLayer(renderLayerDebugDraw);
    
}

RenderLayerManager::~RenderLayerManager()
{
	size_t size = array.size();
	for (size_t i = 0; i < size; ++i)
		SafeDelete(array[i]);
	array.clear();
	map.clear();
}


};