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


#include "Render/RenderManager.h"
#include "Render/OcclusionQuery.h"
#include "Render/Highlevel/StaticOcclusionRenderPass.h"
#include "Render/Highlevel/RenderBatchArray.h"
#include "Render/Highlevel/StaticOcclusion.h"
#include "Utils/StringFormat.h"

namespace DAVA
{

StaticOcclusionRenderPass::StaticOcclusionRenderPass(const FastName & name, StaticOcclusion * _occlusion, RenderPassID id)
    : RenderPass(name, id)
    , occlusion(_occlusion)
{
    
    AddRenderLayer(new RenderLayer(LAYER_OPAQUE, RenderLayerBatchArray::SORT_ENABLED | RenderLayerBatchArray::SORT_BY_DISTANCE_FRONT_TO_BACK, RENDER_LAYER_OPAQUE_ID), LAST_LAYER);
    AddRenderLayer(new RenderLayer(LAYER_AFTER_OPAQUE, RenderLayerBatchArray::SORT_ENABLED | RenderLayerBatchArray::SORT_BY_DISTANCE_FRONT_TO_BACK, RENDER_LAYER_AFTER_OPAQUE_ID), LAST_LAYER);
    AddRenderLayer(new RenderLayer(LAYER_ALPHA_TEST_LAYER, RenderLayerBatchArray::SORT_ENABLED | RenderLayerBatchArray::SORT_BY_DISTANCE_FRONT_TO_BACK, RENDER_LAYER_ALPHA_TEST_LAYER_ID), LAST_LAYER);
    AddRenderLayer(new RenderLayer(LAYER_WATER, RenderLayerBatchArray::SORT_ENABLED | RenderLayerBatchArray::SORT_BY_DISTANCE_FRONT_TO_BACK, RENDER_LAYER_WATER_ID), LAST_LAYER);    
    AddRenderLayer(new RenderLayer(LAYER_TRANSLUCENT, RenderLayerBatchArray::SORT_ENABLED | RenderLayerBatchArray::SORT_BY_DISTANCE_FRONT_TO_BACK, RENDER_LAYER_TRANSLUCENT_ID), LAST_LAYER);
    AddRenderLayer(new RenderLayer(LAYER_AFTER_TRANSLUCENT, RenderLayerBatchArray::SORT_ENABLED | RenderLayerBatchArray::SORT_BY_DISTANCE_FRONT_TO_BACK, RENDER_LAYER_AFTER_TRANSLUCENT_ID), LAST_LAYER);
    
    renderPassBatchArray->InitPassLayers(this);
}
    
StaticOcclusionRenderPass::~StaticOcclusionRenderPass()
{
    
}




    
bool StaticOcclusionRenderPass::CompareFunction(const RenderBatch * a, const RenderBatch *  b)
{
    return a->layerSortingKey < b->layerSortingKey;
}
    
void StaticOcclusionRenderPass::Draw(RenderSystem * renderSystem, uint32 clearBuffers)
{
    Camera *mainCamera = occlusionCamera;
    Camera *drawCamera = occlusionCamera;

    DVASSERT(drawCamera);
    DVASSERT(mainCamera);
    drawCamera->SetupDynamicParameters();            
    if (mainCamera!=drawCamera)    
        mainCamera->PrepareDynamicParameters();

    PrepareVisibilityArrays(mainCamera, renderSystem);

    ClearBuffers(clearBuffers);
	
    Vector<RenderBatch*> terrainBatches;
    Vector<RenderBatch*> batches;
    
    terrainBatches.clear();
    batches.clear();
    
    uint32 size = (uint32)renderLayers.size();
    for (uint32 k = 0; k < size; ++k)
    {
        RenderLayer * layer = renderLayers[k];
        RenderLayerBatchArray * renderLayerBatchArray = renderPassBatchArray->Get(layer->GetRenderLayerID());
    
        uint32 batchCount = (uint32)renderLayerBatchArray->GetRenderBatchCount();
        for (uint32 batchIndex = 0; batchIndex < batchCount; ++batchIndex)
        {
            RenderBatch * batch = renderLayerBatchArray->Get(batchIndex);
            if (batch->GetRenderObject()->GetType() == RenderObject::TYPE_LANDSCAPE)
            {
                terrainBatches.push_back(batch);
            }else
            {
                batches.push_back(batch);
            }
        }
    }
    
    //glColorMask(false, false, false, false);
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
    
//    size = (uint32)terrainBatches.size();
//    for (uint32 k = 0; k < size; ++k)
//    {
//        RenderBatch * batch = terrainBatches[k];
//        batch->Draw(name, camera);
//    }
//    
//    size = (uint32)batches.size();
//    for (uint32 k = 0; k < size; ++k)
//    {
//        RenderBatch * batch = batches[k];
//        batch->Draw(name, camera);
//    }
    
    //
//    glDepthFunc(GL_LEQUAL);
//    glDepthMask(GL_FALSE);

    OcclusionQueryPool & manager = occlusion->GetOcclusionQueryPool();
    size = (uint32)terrainBatches.size();
    for (uint32 k = 0; k < size; ++k)
    {
        OcclusionQueryPoolHandle handle = manager.CreateQueryObject();
        OcclusionQuery & query = manager.Get(handle);
        
        RenderBatch * batch = terrainBatches[k];
        
        query.BeginQuery();
        batch->Draw(name, mainCamera);
        query.EndQuery();
        
        occlusion->RecordFrameQuery(batch, handle);
    }
    
    size = (uint32)batches.size();
    for (uint32 k = 0; k < size; ++k)
    {
        OcclusionQueryPoolHandle handle = manager.CreateQueryObject();
        OcclusionQuery & query = manager.Get(handle);
        
        RenderBatch * batch = batches[k];
        
        query.BeginQuery();
        batch->Draw(name, mainCamera);
        query.EndQuery();
        
        //glFlush();
        
        
//        while (!query.IsResultAvailable())
//        {
//        }
//        uint32 result;
//        query.GetQuery(&result);
//        
//        if ((debugK) && (debugI == 0) && (debugJ == 0))
//        {
//            //RenderManager::Instance()->SetRenderTarget((Sprite*)0);
//            const Matrix4 * oldProj = (const Matrix4*)RenderManager::Instance()->GetDynamicParam(PARAM_PROJ);
//            Image * image = occlusion->GetRTTexture()->CreateImageFromMemory(RenderState::RENDERSTATE_3D_BLEND);
//            ImageLoader::Save(image, Format("~doc:/renderobj_%d_%d_%d.png", debugSide, k, result));
//            SafeRelease(image);
//            RenderManager::Instance()->SetDynamicParam(PARAM_PROJ, oldProj, UPDATE_SEMANTIC_ALWAYS);
// 
//            //RenderManager::Instance()->RestoreRenderTarget();
//        }
//
//        if ((debugK) && (debugI == 1) && (debugJ == 0))
//        {
//            const Matrix4 * oldProj = (const Matrix4*)RenderManager::Instance()->GetDynamicParam(PARAM_PROJ);
//            Image * image = occlusion->GetRTTexture()->CreateImageFromMemory(RenderState::RENDERSTATE_2D_OPAQUE);
//            ImageLoader::Save(image, Format("~doc:/renderobj2_%d_%d.png", debugSide, k));
//            SafeRelease(image);
//            RenderManager::Instance()->SetDynamicParam(PARAM_PROJ, oldProj, UPDATE_SEMANTIC_ALWAYS);
//        }
        
//        if ((debugK) && (debugI == 1) && (debugJ == 1))
//        {
//            RenderManager::Instance()->SetRenderTarget((Sprite*)0);
//            Image * image = occlusion->GetRTTexture()->CreateImageFromMemory(RenderState::RENDERSTATE_2D_OPAQUE);
//            ImageLoader::Save(image, Format("~doc:/renderobj3_%d_%d.png", debugSide, k));
//            SafeRelease(image);
//            RenderManager::Instance()->RestoreRenderTarget();
//        }
//        
//        if ((debugK) && (debugI == 5) && (debugJ == 5))
//        {
//            RenderManager::Instance()->SetRenderTarget((Sprite*)0);
//            Image * image = occlusion->GetRTTexture()->CreateImageFromMemory(RenderState::RENDERSTATE_2D_OPAQUE);
//            ImageLoader::Save(image, Format("~doc:/renderobj4_%d_%d.png", debugSide, k));
//            SafeRelease(image);
//            RenderManager::Instance()->RestoreRenderTarget();
//        }
        
        occlusion->RecordFrameQuery(batch, handle);
    }
    
    //
//    glDepthMask(GL_TRUE);
//    glDepthFunc(GL_LESS);
//    glColorMask(true, true, true, true);
}

};
