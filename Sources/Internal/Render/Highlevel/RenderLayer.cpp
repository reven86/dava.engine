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

void RenderLayer::Draw(Camera* camera, RenderLayerBatchArray * renderLayerBatchArray, rhi::HPacketList packetList)
{
    TIME_PROFILE("RenderLayer::Draw");
    
    renderLayerBatchArray->Sort(camera);
    
    uint32 size = (uint32)renderLayerBatchArray->GetRenderBatchCount();

    FrameOcclusionQueryManager::Instance()->BeginQuery(name);
    rhi::Packet packet;
    
    for (uint32 k = 0; k < size; ++k)
    {
        RenderBatch* batch = renderLayerBatchArray->Get(k);
        RenderObject* renderObject = batch->GetRenderObject();
        DVASSERT(renderObject != 0);
        renderObject->BindDynamicParameters(camera);
        
        PolygonGroup *pg = batch->GetPolygonGroup();
        NMaterial *mat = batch->GetMaterial();
        if (pg && mat)
        {                     
            packet.vertexStreamCount = 1;
            packet.vertexStream[0] = pg->vertexBuffer;
            packet.indexBuffer = pg->indexBuffer;
            packet.primitiveType = rhi::PRIMITIVE_TRIANGLELIST; //RHI_COMPLETE
            packet.primitiveCount = pg->indexCount/3;
            packet.vertexLayoutUID = pg->vertexLayoutId;
            packet.vertexCount = pg->vertexCount;
            DVASSERT(packet.primitiveCount);

            mat->BindParams(packet);

            rhi::AddPacket(packetList, packet);
        }        

    }
    
    FrameOcclusionQueryManager::Instance()->EndQuery(name);
    
}


};
