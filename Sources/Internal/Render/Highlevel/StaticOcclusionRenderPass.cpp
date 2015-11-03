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

#include "FileSystem/FileSystem.h"
#include "Render/Image/Image.h"
#include "Render/RenderCallbacks.h"
#include "Render/ShaderCache.h"
#include "Render/OcclusionQuery.h"
#include "Render/Highlevel/StaticOcclusionRenderPass.h"
#include "Render/Highlevel/RenderBatchArray.h"
#include "Render/Highlevel/StaticOcclusion.h"
#include "Utils/StringFormat.h"

namespace DAVA
{
const uint32 OCCLUSION_RENDER_TARGET_SIZE = 1024;

StaticOcclusionRenderPass::StaticOcclusionRenderPass(const FastName& name)
    : RenderPass(name)
{
    meshBatchesWithDepthWriteOption.reserve(1024);
    terrainBatches.reserve(256);

    uint32 sortingFlags = RenderBatchArray::SORT_THIS_FRAME | RenderBatchArray::SORT_BY_DISTANCE_FRONT_TO_BACK;
    AddRenderLayer(new RenderLayer(RenderLayer::RENDER_LAYER_OPAQUE_ID, sortingFlags));
    AddRenderLayer(new RenderLayer(RenderLayer::RENDER_LAYER_AFTER_OPAQUE_ID, sortingFlags));
    AddRenderLayer(new RenderLayer(RenderLayer::RENDER_LAYER_WATER_ID, sortingFlags));

    rhi::Texture::Descriptor descriptor;

    descriptor.isRenderTarget = 1;
    descriptor.width = OCCLUSION_RENDER_TARGET_SIZE;
    descriptor.height = OCCLUSION_RENDER_TARGET_SIZE;
    descriptor.autoGenMipmaps = false;
    descriptor.type = rhi::TEXTURE_TYPE_2D;
    descriptor.format = rhi::TEXTURE_FORMAT_R8G8B8A8;
    colorBuffer = rhi::CreateTexture(descriptor);

    descriptor.isRenderTarget = 0;
    descriptor.format = rhi::TEXTURE_FORMAT_D24S8;
    depthBuffer = rhi::CreateTexture(descriptor);

    passConfig.colorBuffer[0].texture = colorBuffer;
    passConfig.colorBuffer[0].loadAction = rhi::LOADACTION_NONE;
    passConfig.colorBuffer[0].storeAction = rhi::STOREACTION_NONE;

    passConfig.depthStencilBuffer.texture = depthBuffer;
    passConfig.depthStencilBuffer.loadAction = rhi::LOADACTION_CLEAR;
    passConfig.depthStencilBuffer.storeAction = rhi::STOREACTION_NONE;

    passConfig.viewport.width = OCCLUSION_RENDER_TARGET_SIZE;
    passConfig.viewport.height = OCCLUSION_RENDER_TARGET_SIZE;
    passConfig.priority = PRIORITY_SERVICE_3D;

    /*
	 * Create depth states
	 */
    rhi::DepthStencilState::Descriptor ds;

    ds.depthWriteEnabled = 0;
    depthWriteStateState[0] = rhi::AcquireDepthStencilState(ds);

    ds.depthWriteEnabled = 1;
    depthWriteStateState[1] = rhi::AcquireDepthStencilState(ds);
}

StaticOcclusionRenderPass::~StaticOcclusionRenderPass()
{
    rhi::DeleteTexture(colorBuffer);
    rhi::DeleteTexture(depthBuffer);
    rhi::ReleaseDepthStencilState(depthWriteStateState[0]);
    rhi::ReleaseDepthStencilState(depthWriteStateState[1]);
}

#if (SAVE_OCCLUSION_IMAGES)

rhi::HTexture sharedColorBuffer = rhi::HTexture(rhi::InvalidHandle);
Map<rhi::HSyncObject, String> renderPassFileNames;

void OnOcclusionRenderPassCompleted(rhi::HSyncObject syncObj)
{
    DVASSERT(renderPassFileNames.count(syncObj) > 0);
    DVASSERT(sharedColorBuffer != rhi::HTexture(rhi::InvalidHandle));

    void* data = rhi::MapTexture(sharedColorBuffer, 0);

    Image* img = Image::CreateFromData(OCCLUSION_RENDER_TARGET_SIZE_X, OCCLUSION_RENDER_TARGET_SIZE_Y,
                                       PixelFormat::FORMAT_RGBA8888, reinterpret_cast<uint8*>(data));
    img->Save(renderPassFileNames.at(syncObj));
    SafeRelease(img);

    rhi::UnmapTexture(sharedColorBuffer);
    rhi::DeleteSyncObject(syncObj);
}
#endif

bool StaticOcclusionRenderPass::ShouldEnableDepthWriteForRenderObject(RenderObject* ro)
{
    auto it = switchRenderObjects.find(ro);
    if (it != switchRenderObjects.end())
    {
        return it->second;
    }

    auto count = ro->GetRenderBatchCount();
    for (uint32 i = 0; i < count; ++i)
    {
        int32 switchIndex = -1;
        int32 lodIndex = -1;
        ro->GetRenderBatch(i, lodIndex, switchIndex);
        if (switchIndex > 0)
        {
            switchRenderObjects.insert({ ro, false });
            return false;
        }
    }

    switchRenderObjects.insert({ ro, true });
    return true;
}

void StaticOcclusionRenderPass::DrawOcclusionFrame(RenderSystem* renderSystem, Camera* occlusionCamera,
                                                   StaticOcclusionFrameResult& target, const StaticOcclusionData& data,
                                                   uint32 blockIndex)
{
    meshBatchesWithDepthWriteOption.clear();
    terrainBatches.clear();

    ShaderDescriptorCache::ClearDynamicBindigs();
    SetupCameraParams(occlusionCamera, occlusionCamera);
    PrepareVisibilityArrays(occlusionCamera, renderSystem);

    std::unordered_set<uint32> invisibleObjects;
    Vector3 cameraPosition = occlusionCamera->GetPosition();

    for (uint32 k = 0, size = (uint32)renderLayers.size(); k < size; ++k)
    {
        RenderLayer * layer = renderLayers[k];
        const RenderBatchArray& renderBatchArray = layersBatchArrays[layer->GetRenderLayerID()];

        uint32 batchCount = (uint32)renderBatchArray.GetRenderBatchCount();
        for (uint32 batchIndex = 0; batchIndex < batchCount; ++batchIndex)
        {
            RenderBatch* batch = renderBatchArray.Get(batchIndex);
            auto renderObject = batch->GetRenderObject();
            auto objectType = renderObject->GetType();

            if (objectType == RenderObject::TYPE_LANDSCAPE)
            {
                terrainBatches.push_back(batch);
            }
            else if (objectType != RenderObject::TYPE_PARTICLE_EMTITTER)
            {
                bool shouldEnableDepthWrite = ShouldEnableDepthWriteForRenderObject(renderObject);
                auto option = shouldEnableDepthWrite ? Option_DepthWriteEnabled : Option_DepthWriteDisabled;
                meshBatchesWithDepthWriteOption.emplace_back(batch, option);

                Vector3 position = renderObject->GetWorldBoundingBox().GetCenter();
                batch->layerSortingKey = ((uint32)((position - cameraPosition).SquareLength() * 100.0f));

                auto occlusionId = batch->GetRenderObject()->GetStaticOcclusionIndex();
                bool occlusionIndexIsInvalid = occlusionId == INVALID_STATIC_OCCLUSION_INDEX;
                bool isAlreadyVisible = occlusionIndexIsInvalid || data.IsObjectVisibleFromBlock(blockIndex, occlusionId);
                if (!isAlreadyVisible)
                {
                    invisibleObjects.insert(occlusionId);
                }
            }
        }
    }

    if (invisibleObjects.empty())
        return;

    std::sort(meshBatchesWithDepthWriteOption.begin(), meshBatchesWithDepthWriteOption.end(),
              [](const RenderBatchWithDepthOption& a, const RenderBatchWithDepthOption& b) { return a.first->layerSortingKey < b.first->layerSortingKey; });

    target.blockIndex = blockIndex;
    target.queryBuffer = rhi::CreateQueryBuffer(static_cast<uint32>(meshBatchesWithDepthWriteOption.size()));
    target.frameRequests.resize(meshBatchesWithDepthWriteOption.size(), nullptr);

    passConfig.queryBuffer = target.queryBuffer;
    renderPass = rhi::AllocateRenderPass(passConfig, 1, &packetList);
    rhi::BeginRenderPass(renderPass);
    rhi::BeginPacketList(packetList);

    for (auto batch : terrainBatches)
    {
        rhi::Packet packet;
        RenderObject* renderObject = batch->GetRenderObject();
        renderObject->BindDynamicParameters(occlusionCamera);
        NMaterial* mat = batch->GetMaterial();
        DVASSERT(mat);
        batch->BindGeometryData(packet);
        DVASSERT(packet.primitiveCount);
        mat->BindParams(packet);
        packet.cullMode = rhi::CULL_NONE;
        rhi::AddPacket(packetList, packet);
    }

    uint16 k = 0;
    for (const auto& batch : meshBatchesWithDepthWriteOption)
    {
        RenderObject* renderObject = batch.first->GetRenderObject();
        renderObject->BindDynamicParameters(occlusionCamera);

        rhi::Packet packet;
        batch.first->BindGeometryData(packet);
        DVASSERT(packet.primitiveCount);
        batch.first->GetMaterial()->BindParams(packet);
        if (invisibleObjects.count(renderObject->GetStaticOcclusionIndex()) > 0)
        {
            packet.queryIndex = k;
            target.frameRequests[packet.queryIndex] = renderObject;
        }
        packet.cullMode = rhi::CULL_NONE;
        packet.depthStencilState = depthWriteStateState[batch.second];
        rhi::AddPacket(packetList, packet);
        ++k;
    }

#if (SAVE_OCCLUSION_IMAGES)
    auto syncObj = rhi::CreateSyncObject();

    auto pos = occlusionCamera->GetPosition();
    auto dir = occlusionCamera->GetDirection();
    auto folder = DAVA::Format("~doc:/occlusion/block-%03d", blockIndex);
    FileSystem::Instance()->CreateDirectoryW(FilePath(folder), true);
    auto fileName = DAVA::Format("/[%d,%d,%d] from (%d,%d,%d).png",
                                 int32(dir.x), int32(dir.y), int32(dir.z), int32(pos.x), int32(pos.y), int32(pos.z));
    renderPassFileNames.insert({ syncObj, folder + fileName });
    sharedColorBuffer = colorBuffer;

    RenderCallbacks::RegisterSyncCallback(syncObj, &OnOcclusionRenderPassCompleted);
    rhi::EndPacketList(packetList, syncObj);
    rhi::EndRenderPass(renderPass);
#else

    rhi::EndPacketList(packetList);
    rhi::EndRenderPass(renderPass);

#endif
}
};
