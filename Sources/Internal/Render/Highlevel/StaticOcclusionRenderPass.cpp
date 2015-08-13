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
#include "Render/OcclusionQuery.h"
#include "Render/Highlevel/StaticOcclusionRenderPass.h"
#include "Render/Highlevel/RenderBatchArray.h"
#include "Render/Highlevel/StaticOcclusion.h"
#include "Utils/StringFormat.h"

namespace DAVA
{

StaticOcclusionRenderPass::StaticOcclusionRenderPass(const FastName & name, StaticOcclusion * _occlusion)
    : RenderPass(name)
    , occlusion(_occlusion)
{
    uint32 sortingFlags = RenderBatchArray::SORT_ENABLED | RenderBatchArray::SORT_BY_DISTANCE_BACK_TO_FRONT;
    AddRenderLayer(new RenderLayer(RenderLayer::RENDER_LAYER_OPAQUE_ID, sortingFlags));
    AddRenderLayer(new RenderLayer(RenderLayer::RENDER_LAYER_AFTER_OPAQUE_ID, sortingFlags));
    AddRenderLayer(new RenderLayer(RenderLayer::RENDER_LAYER_ALPHA_TEST_LAYER_ID, sortingFlags));
    AddRenderLayer(new RenderLayer(RenderLayer::RENDER_LAYER_WATER_ID, sortingFlags));
    AddRenderLayer(new RenderLayer(RenderLayer::RENDER_LAYER_TRANSLUCENT_ID, sortingFlags));
    AddRenderLayer(new RenderLayer(RenderLayer::RENDER_LAYER_AFTER_TRANSLUCENT_ID, sortingFlags));
}

StaticOcclusionRenderPass::~StaticOcclusionRenderPass()
{
    
}

bool StaticOcclusionRenderPass::CompareFunction(const RenderBatch * a, const RenderBatch *  b)
{
    return a->layerSortingKey < b->layerSortingKey;
}
    
void StaticOcclusionRenderPass::Draw(RenderSystem * renderSystem)
{
    Camera *mainCamera = occlusionCamera;
    Camera *drawCamera = occlusionCamera;

    SetupCameraParams(mainCamera, drawCamera);

    PrepareVisibilityArrays(mainCamera, renderSystem);    
	
    Vector<RenderBatch*> terrainBatches;
    Vector<RenderBatch*> batches;
    
    terrainBatches.clear();
    batches.clear();
    
    uint32 size = (uint32)renderLayers.size();
    for (uint32 k = 0; k < size; ++k)
    {
        RenderLayer * layer = renderLayers[k];
        const RenderBatchArray & renderBatchArray = layersBatchArrays[layer->GetRenderLayerID()];
    
        uint32 batchCount = (uint32)renderBatchArray.GetRenderBatchCount();
        for (uint32 batchIndex = 0; batchIndex < batchCount; ++batchIndex)
        {
            RenderBatch * batch = renderBatchArray.Get(batchIndex);
            if (batch->GetRenderObject()->GetType() == RenderObject::TYPE_LANDSCAPE)
            {
                terrainBatches.push_back(batch);
            }
            else
            {
                batches.push_back(batch);
            }
        }
    }
        
    // Sort
    Vector3 cameraPosition = mainCamera->GetPosition();
    
    size = (uint32)batches.size();
    for (uint32 k = 0; k < size; ++k)
    {
        RenderBatch * batch = batches[k];
        RenderObject * renderObject = batch->GetRenderObject();
        Vector3 position = renderObject->GetWorldBoundingBox().GetCenter();
        float realDistance = (position - cameraPosition).Length();
        uint32 distance = ((uint32)(realDistance * 100.0f));
        uint32 distanceBits = distance;
        
        batch->layerSortingKey = distanceBits;
    }
    std::sort(batches.begin(), batches.end(), CompareFunction);
    


    OcclusionQueryPool & manager = occlusion->GetOcclusionQueryPool();

    rhi::Packet packet;
    size = (uint32)terrainBatches.size();
    for (uint32 k = 0; k < size; ++k)
    {
        OcclusionQueryPoolHandle handle = manager.CreateQueryObject();
        OcclusionQuery & query = manager.Get(handle);
        
        RenderBatch * batch = terrainBatches[k];
        
        RenderObject* renderObject = batch->GetRenderObject();
        renderObject->BindDynamicParameters(mainCamera);
        NMaterial *mat = batch->GetMaterial();
        DVASSERT(mat); //bad idea to have landscape not staged for render pass
        batch->BindGeometryData(packet);
        DVASSERT(packet.primitiveCount);
        mat->BindParams(packet);
        rhi::AddPacket(packetList, packet);                                
    }
    
    size = (uint32)batches.size();
    for (uint32 k = 0; k < size; ++k)
    {
        OcclusionQueryPoolHandle handle = manager.CreateQueryObject();
        OcclusionQuery & query = manager.Get(handle);
        
        RenderBatch * batch = batches[k];
        RenderObject* renderObject = batch->GetRenderObject();
        renderObject->BindDynamicParameters(mainCamera);
        NMaterial *mat = batch->GetMaterial();
        if (mat)
        {
            batch->BindGeometryData(packet);
            DVASSERT(packet.primitiveCount);
            mat->BindParams(packet);            
            rhi::AddPacket(packetList, packet);
        }
        /*query.BeginQuery();
        batch->Draw(name, mainCamera);
        query.EndQuery();                       */
        
        occlusion->RecordFrameQuery(batch, handle);
    }

}

};
