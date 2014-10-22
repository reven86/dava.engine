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


#include "Render/Highlevel/RenderLayer.h"
#include "Render/Highlevel/RenderBatch.h"
#include "Render/Highlevel/RenderBatchArray.h"
#include "Render/Highlevel/Camera.h"
#include "Base/Radix/Radix.h"
#include "Debug/Stats.h"
#include "Render/OcclusionQuery.h"

namespace DAVA
{
RenderLayer::RenderLayer(const FastName & _name, uint32 sortingFlags, RenderLayerID _id)
    :	name(_name)
    ,   flags(sortingFlags)
    ,   id(_id)
{
    
}
    
RenderLayer::~RenderLayer()
{
}

void RenderLayer::Draw(const FastName & ownerRenderPass, Camera * camera, RenderLayerBatchArray * renderLayerBatchArray)
{
    TIME_PROFILE("RenderLayer::Draw");
    
    renderLayerBatchArray->Sort(camera);
    
#if CAN_INSTANCE_CHECK
    RenderBatch * prevBatch = 0;
    uint32 canBeInstanced = 0;
    
    Vector<int32> chain;
#endif
    uint32 size = (uint32)renderLayerBatchArray->GetRenderBatchCount();

    FrameOcclusionQueryManager::Instance()->BeginQuery(name);
    
    for (uint32 k = 0; k < size; ++k)
    {
        RenderBatch * batch = renderLayerBatchArray->Get(k);

#if CAN_INSTANCE_CHECK
        if (prevBatch && batch->GetPolygonGroup() == prevBatch->GetPolygonGroup() && batch->GetMaterial()->GetParent() == prevBatch->GetMaterial()->GetParent())
        {
            canBeInstanced++;
        }else
        {
            if (canBeInstanced > 0)
                chain.push_back(canBeInstanced + 1);
            canBeInstanced = 0;
        }
#endif
        batch->Draw(ownerRenderPass, camera);
#if CAN_INSTANCE_CHECK
        prevBatch = batch;
#endif
    }
    
    FrameOcclusionQueryManager::Instance()->EndQuery(name);
    
#if CAN_INSTANCE_CHECK
    int32 realDrawEconomy = 0;
    for (uint32 k = 0; k < chain.size(); ++k)
    {
        realDrawEconomy += (chain[k] - 1);
    }
    
    Logger::Debug("Pass: %s Layer: %s Size: %d Can be instanced: %d Econ: %d", ownerRenderPass.c_str(), name.c_str(), size, chain.size(), realDrawEconomy);
    for (uint32 k = 0; k < chain.size(); ++k)
    {
        Logger::Debug("%d - %d", k, chain[k]);
    }
#endif
}


};
