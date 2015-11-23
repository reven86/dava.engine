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
const FastName LAYER_NAME_OPAQUE("OpaqueRenderLayer");
const FastName LAYER_NAME_AFTER_OPAQUE("AfterOpaqueRenderLayer");
const FastName LAYER_NAME_ALPHA_TEST_LAYER("AlphaTestLayer");
const FastName LAYER_NAME_WATER("WaterLayer");
const FastName LAYER_NAME_TRANSLUCENT("TransclucentRenderLayer");
const FastName LAYER_NAME_AFTER_TRANSLUCENT("AfterTransclucentRenderLayer");
const FastName LAYER_NAME_SHADOW_VOLUME("ShadowVolumeRenderLayer");
const FastName LAYER_NAME_VEGETATION("VegetationRenderLayer");
const FastName LAYER_NAME_DEBUG_DRAW("DebugRenderLayer");

const FastName LAYER_NAMES[RenderLayer::RENDER_LAYER_ID_COUNT] =
{
  LAYER_NAME_OPAQUE,
  LAYER_NAME_AFTER_OPAQUE,
  LAYER_NAME_ALPHA_TEST_LAYER,
  LAYER_NAME_WATER,
  LAYER_NAME_TRANSLUCENT,
  LAYER_NAME_AFTER_TRANSLUCENT,
  LAYER_NAME_SHADOW_VOLUME,
  LAYER_NAME_VEGETATION,
  LAYER_NAME_DEBUG_DRAW
};

const uint32 RenderLayer::LAYER_SORTING_FLAGS_OPAQUE = RenderBatchArray::SORT_ENABLED | RenderBatchArray::SORT_BY_MATERIAL;
const uint32 RenderLayer::LAYER_SORTING_FLAGS_AFTER_OPAQUE = RenderBatchArray::SORT_ENABLED | RenderBatchArray::SORT_BY_MATERIAL;
const uint32 RenderLayer::LAYER_SORTING_FLAGS_ALPHA_TEST_LAYER = RenderBatchArray::SORT_ENABLED | RenderBatchArray::SORT_BY_MATERIAL;
const uint32 RenderLayer::LAYER_SORTING_FLAGS_WATER = 0;
const uint32 RenderLayer::LAYER_SORTING_FLAGS_TRANSLUCENT = RenderBatchArray::SORT_ENABLED | RenderBatchArray::SORT_BY_DISTANCE_BACK_TO_FRONT;
const uint32 RenderLayer::LAYER_SORTING_FLAGS_AFTER_TRANSLUCENT = RenderBatchArray::SORT_ENABLED | RenderBatchArray::SORT_BY_MATERIAL;
const uint32 RenderLayer::LAYER_SORTING_FLAGS_SHADOW_VOLUME = 0;
const uint32 RenderLayer::LAYER_SORTING_FLAGS_VEGETATION = 0;
const uint32 RenderLayer::LAYER_SORTING_FLAGS_DEBUG_DRAW = 0;

RenderLayer::RenderLayer(eRenderLayerID _id, uint32 sortingFlags)
    : layerID(_id)
    , sortFlags(sortingFlags)
{
}

RenderLayer::~RenderLayer()
{
}

const FastName& RenderLayer::GetLayerNameByID(eRenderLayerID layer)
{
    DVASSERT(layer >= 0 && layer < RENDER_LAYER_ID_COUNT);
    return LAYER_NAMES[layer];
}

RenderLayer::eRenderLayerID RenderLayer::GetLayerIDByName(const FastName& name)
{
    for (int32 id = 0; id < RENDER_LAYER_ID_COUNT; ++id)
    {
        if (LAYER_NAMES[id] == name)
        {
            return (eRenderLayerID)id;
        }
    }
    return RENDER_LAYER_INVALID_ID;
}

void RenderLayer::Draw(Camera* camera, const RenderBatchArray& batchArray, rhi::HPacketList packetList)
{
    TIME_PROFILE("RenderLayer::Draw");

    uint32 size = (uint32)batchArray.GetRenderBatchCount();

    FrameOcclusionQueryManager::Instance()->BeginQuery(GetLayerNameByID(layerID));
    rhi::Packet packet;

    for (uint32 k = 0; k < size; ++k)
    {
        RenderBatch* batch = batchArray.Get(k);
        RenderObject* renderObject = batch->GetRenderObject();
        renderObject->BindDynamicParameters(camera);
        NMaterial* mat = batch->GetMaterial();
        if (mat)
        {
            batch->BindGeometryData(packet);
            DVASSERT(packet.primitiveCount);
            mat->BindParams(packet);
            packet.debugMarker = mat->GetEffectiveFXName().c_str();
            rhi::AddPacket(packetList, packet);
        }
    }

    FrameOcclusionQueryManager::Instance()->EndQuery(GetLayerNameByID(layerID));
}
};
