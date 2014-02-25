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
    
StaticOcclusionRenderLayer::StaticOcclusionRenderLayer(const FastName & name, uint32 sortingFlags, StaticOcclusion * _occlusion, RenderLayerID id)
    : RenderLayer(name, sortingFlags, id)
    , occlusion(_occlusion)
{
    
}
StaticOcclusionRenderLayer::~StaticOcclusionRenderLayer()
{
    
}

void StaticOcclusionRenderLayer::Draw(const FastName & ownerRenderPass, Camera * camera, RenderLayerBatchArray * renderLayerBatchArray)
{
    renderLayerBatchArray->Sort(camera);
    uint32 size = (uint32)renderLayerBatchArray->GetRenderBatchCount();
    
    OcclusionQueryManager & manager = occlusion->GetOcclusionQueryManager();
    
    for (uint32 k = 0; k < size; ++k)
    {
        OcclusionQueryManagerHandle handle = manager.CreateQueryObject();
        OcclusionQuery & query = manager.Get(handle);
        
        RenderBatch * batch = renderLayerBatchArray->Get(k);
        
        query.BeginQuery();
        batch->Draw(ownerRenderPass, camera);
        query.EndQuery();
        
        occlusion->RecordFrameQuery(batch, handle);
    }
    //Logger::FrameworkDebug(Format("Pass: %s Layer: %s - objects: %d", ownerRenderPass.c_str(), name.c_str(), size));
}

StaticOcclusionRenderPass::StaticOcclusionRenderPass(RenderSystem * rs, const FastName & name, StaticOcclusion * _occlusion, RenderPassID id)
    : RenderPass(rs, name, id)
    , occlusion(_occlusion)
{
}
    
StaticOcclusionRenderPass::~StaticOcclusionRenderPass()
{
    
}

void StaticOcclusionRenderPass::Draw(Camera * camera, RenderPassBatchArray * renderPassBatchArray)
{
    uint32 size = (uint32)renderLayers.size();
    for (uint32 k = 0; k < size; ++k)
    {
        RenderLayer * layer = renderLayers[k];
        RenderLayerBatchArray * renderLayerBatchArray = renderPassBatchArray->Get(layer->GetRenderLayerID());
        if (renderLayerBatchArray)
        {
            layer->Draw(name, camera, renderLayerBatchArray);
        }
    }
}



};
