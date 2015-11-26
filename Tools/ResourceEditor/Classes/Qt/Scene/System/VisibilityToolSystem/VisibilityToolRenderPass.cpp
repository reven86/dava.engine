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

#include "VisibilityToolRenderPass.h"
#include "Render/ShaderCache.h"

using namespace DAVA;

const uint32 cubemapFaces = 6;

VisibilityToolRenderPass::VisibilityToolRenderPass()
    : RenderPass(PASS_FORWARD)
    , camera(new Camera())
    , distanceMaterial(new NMaterial())
    , overrideMaterial(new NMaterial())
{
    camera->SetupPerspective(90.0f, 1.0f, 1.0f, 5000.0f);

    auto sortingFlags = RenderBatchArray::SORT_THIS_FRAME | RenderBatchArray::SORT_BY_DISTANCE_BACK_TO_FRONT;
    AddRenderLayer(new RenderLayer(RenderLayer::RENDER_LAYER_OPAQUE_ID, sortingFlags));
    AddRenderLayer(new RenderLayer(RenderLayer::RENDER_LAYER_AFTER_OPAQUE_ID, sortingFlags));

    renderTargetConfig.colorBuffer[0].loadAction = rhi::LOADACTION_CLEAR;
    renderTargetConfig.depthStencilBuffer.loadAction = rhi::LOADACTION_CLEAR;
    renderTargetConfig.priority = PRIORITY_SERVICE_3D;
    renderTargetConfig.viewport.x = 0;
    renderTargetConfig.viewport.y = 0;
    std::fill_n(renderTargetConfig.colorBuffer[0].clearColor, 4, 1.0f);

    rhi::DepthStencilState::Descriptor dsDesc;
    dsDesc.depthFunc = rhi::CMP_LESSEQUAL;
    dsDesc.depthTestEnabled = 1;
    dsDesc.depthWriteEnabled = 0;
    overrideDepthStencilState = rhi::AcquireDepthStencilState(dsDesc);

    overrideConfig.colorBuffer[0].loadAction = rhi::LOADACTION_LOAD;
    overrideConfig.depthStencilBuffer.loadAction = rhi::LOADACTION_LOAD;
    overrideConfig.priority = PRIORITY_MAIN_3D - 5;

    distanceMaterial->SetFXName(FastName("~res:/LandscapeEditor/Materials/Distance.Opaque.material"));
    distanceMaterial->PreBuildMaterial(PASS_FORWARD);

    overrideMaterial->SetFXName(FastName("~res:/LandscapeEditor/Materials/CompareDistance.Opaque.material"));
    overrideMaterial->AddFlag(NMaterialFlagName::FLAG_BLENDING, BLENDING_ADDITIVE);
    overrideMaterial->PreBuildMaterial(PASS_FORWARD);
}

VisibilityToolRenderPass::~VisibilityToolRenderPass()
{
}

void VisibilityToolRenderPass::RenderToCubemapFromPoint(RenderSystem* renderSystem, Texture* renderTarget, const Vector3& point)
{
    renderTargetConfig.colorBuffer[0].texture = renderTarget->handle;
    renderTargetConfig.depthStencilBuffer.texture = renderTarget->handleDepthStencil;
    renderTargetConfig.viewport.width = renderTarget->GetWidth();
    renderTargetConfig.viewport.height = renderTarget->GetHeight();

    camera->SetPosition(point);
    for (uint32 i = 0; i < cubemapFaces; ++i)
    {
        SetupCameraToRenderFromPointToFaceIndex(point, i);
        RenderWithCurrentSettings(renderSystem);
    }

    DrawOverrideWithCurrentSettings(renderSystem, renderTarget, point);
}

void VisibilityToolRenderPass::SetupCameraToRenderFromPointToFaceIndex(const Vector3& point, uint32 faceIndex)
{
    const Vector3 directions[cubemapFaces] =
    {
      Vector3(+1.0f, 0.0f, 0.0f),
      Vector3(-1.0f, 0.0f, 0.0f),
      Vector3(0.0f, +1.0f, 0.0f),
      Vector3(0.0f, -1.0f, 0.0f),
      Vector3(0.0f, 0.0f, +1.0f),
      Vector3(0.0f, 0.0f, -1.0f),
    };
    const Vector3 upVectors[cubemapFaces] =
    {
      Vector3(0.0f, -1.0f, 0.0f),
      Vector3(0.0f, -1.0f, 0.0f),
      Vector3(0.0f, 0.0f, 1.0f),
      Vector3(0.0f, 0.0f, -1.0f),
      Vector3(0.0f, -1.0f, 0.0f),
      Vector3(0.0f, -1.0f, 0.0f),
    };

    const rhi::TextureFace targetFaces[cubemapFaces] =
    {
      rhi::TEXTURE_FACE_POSITIVE_X,
      rhi::TEXTURE_FACE_NEGATIVE_X,
      rhi::TEXTURE_FACE_POSITIVE_Y,
      rhi::TEXTURE_FACE_NEGATIVE_Y,
      rhi::TEXTURE_FACE_POSITIVE_Z,
      rhi::TEXTURE_FACE_NEGATIVE_Z,
    };
    renderTargetConfig.colorBuffer[0].cubemapTextureFace = targetFaces[faceIndex];

    camera->SetTarget(point + directions[faceIndex]);
    camera->SetUp(upVectors[faceIndex]);
}

bool VisibilityToolRenderPass::ShouldRenderObject(RenderObject* object)
{
    auto type = object->GetType();

    return (type != RenderObject::TYPE_SPEED_TREE) && (type != RenderObject::TYPE_SPRITE) &&
    (type != RenderObject::TYPE_VEGETATION) && (type != RenderObject::TYPE_PARTICLE_EMTITTER);
}

bool VisibilityToolRenderPass::ShouldRenderBatch(RenderBatch* batch)
{
    return (batch->GetMaterial()->GetEffectiveFXName() != NMaterialName::SKYOBJECT);
}

void VisibilityToolRenderPass::RenderWithCurrentSettings(RenderSystem* renderSystem)
{
    ShaderDescriptorCache::ClearDynamicBindigs();
    SetupCameraParams(camera, camera);
    PrepareVisibilityArrays(camera, renderSystem);

    rhi::HPacketList localPacketList;
    auto renderTargetPass = rhi::AllocateRenderPass(renderTargetConfig, 1, &localPacketList);
    rhi::BeginRenderPass(renderTargetPass);
    rhi::BeginPacketList(localPacketList);
    for (auto layer : renderLayers)
    {
        const RenderBatchArray& renderBatchArray = layersBatchArrays[layer->GetRenderLayerID()];
        uint32 batchCount = (uint32)renderBatchArray.GetRenderBatchCount();
        for (uint32 batchIndex = 0; batchIndex < batchCount; ++batchIndex)
        {
            RenderBatch* batch = renderBatchArray.Get(batchIndex);
            RenderObject* renderObject = batch->GetRenderObject();
            if (ShouldRenderBatch(batch) && ShouldRenderObject(renderObject))
            {
                renderObject->BindDynamicParameters(camera);
                rhi::Packet packet;
                batch->BindGeometryData(packet);
                distanceMaterial->BindParams(packet);
                packet.cullMode = rhi::CULL_NONE;
                rhi::AddPacket(localPacketList, packet);
            }
        }
    }
    rhi::EndPacketList(localPacketList);
    rhi::EndRenderPass(renderTargetPass);
}

void VisibilityToolRenderPass::DrawOverrideWithCurrentSettings(RenderSystem* renderSystem, Texture* renderTarget, const Vector3& point)
{
    FastName fnCubemap("cubemap");
    if (overrideMaterial->HasLocalTexture(fnCubemap))
    {
        overrideMaterial->SetTexture(fnCubemap, renderTarget);
    }
    else
    {
        overrideMaterial->AddTexture(fnCubemap, renderTarget);
    }
    overrideMaterial->PreBuildMaterial(PASS_FORWARD);

    auto mainCamera = renderSystem->GetDrawCamera();

    ShaderDescriptorCache::ClearDynamicBindigs();
    SetupCameraParams(mainCamera, mainCamera);
    PrepareVisibilityArrays(mainCamera, renderSystem);

    auto overridePass = rhi::AllocateRenderPass(overrideConfig, 1, &packetList);
    rhi::BeginRenderPass(overridePass);
    rhi::BeginPacketList(packetList);

    Vector4 lightPosition(point.x, point.y, point.z, 0.0f);

    for (auto layer : renderLayers)
    {
        const RenderBatchArray& renderBatchArray = layersBatchArrays[layer->GetRenderLayerID()];
        uint32 batchCount = (uint32)renderBatchArray.GetRenderBatchCount();
        for (uint32 batchIndex = 0; batchIndex < batchCount; ++batchIndex)
        {
            RenderBatch* batch = renderBatchArray.Get(batchIndex);
            RenderObject* renderObject = batch->GetRenderObject();
            if (ShouldRenderBatch(batch) && ShouldRenderObject(renderObject))
            {
                renderObject->BindDynamicParameters(mainCamera);
                Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_LIGHT0_POSITION, &lightPosition, (pointer_size)&lightPosition);

                rhi::Packet packet;
                batch->BindGeometryData(packet);
                overrideMaterial->BindParams(packet);
                packet.depthStencilState = overrideDepthStencilState;
                rhi::AddPacket(packetList, packet);
            }
        }
    }

    rhi::EndPacketList(packetList);
    rhi::EndRenderPass(overridePass);
}
