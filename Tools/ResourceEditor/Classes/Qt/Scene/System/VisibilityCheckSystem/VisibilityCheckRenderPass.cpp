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

#include "VisibilityCheckRenderPass.h"
#include "Render/ShaderCache.h"

VisibilityCheckRenderPass::VisibilityCheckRenderPass()
    : RenderPass(DAVA::PASS_FORWARD)
    , camera(new DAVA::Camera())
    , distanceMaterial(new DAVA::NMaterial())
    , visibilityMaterial(new DAVA::NMaterial())
    , prerenderMaterial(new DAVA::NMaterial())
{
    camera->SetupPerspective(90.0f, 1.0f, 1.0f, 5000.0f);

    auto sortingFlags = DAVA::RenderBatchArray::SORT_THIS_FRAME | DAVA::RenderBatchArray::SORT_BY_DISTANCE_BACK_TO_FRONT;
    AddRenderLayer(new DAVA::RenderLayer(DAVA::RenderLayer::RENDER_LAYER_OPAQUE_ID, sortingFlags));
    AddRenderLayer(new DAVA::RenderLayer(DAVA::RenderLayer::RENDER_LAYER_AFTER_OPAQUE_ID, sortingFlags));

    prerenderConfig.colorBuffer[0].loadAction = rhi::LOADACTION_CLEAR;
    prerenderConfig.depthStencilBuffer.loadAction = rhi::LOADACTION_CLEAR;
    prerenderConfig.depthStencilBuffer.storeAction = rhi::STOREACTION_STORE;
    prerenderConfig.priority = DAVA::PRIORITY_SERVICE_3D + 5;

    renderTargetConfig.colorBuffer[0].loadAction = rhi::LOADACTION_CLEAR;
    renderTargetConfig.depthStencilBuffer.loadAction = rhi::LOADACTION_CLEAR;
    renderTargetConfig.priority = DAVA::PRIORITY_SERVICE_3D;
    renderTargetConfig.viewport.x = 0;
    renderTargetConfig.viewport.y = 0;
    std::fill_n(renderTargetConfig.colorBuffer[0].clearColor, 4, 1.0f);

    rhi::DepthStencilState::Descriptor dsDesc;
    dsDesc.depthFunc = rhi::CMP_EQUAL;
    dsDesc.depthTestEnabled = 1;
    dsDesc.depthWriteEnabled = 0;
    visibilityDepthStencilState = rhi::AcquireDepthStencilState(dsDesc);

    visibilityConfig.colorBuffer[0].loadAction = rhi::LOADACTION_LOAD;
    visibilityConfig.depthStencilBuffer.loadAction = rhi::LOADACTION_LOAD;
    visibilityConfig.depthStencilBuffer.storeAction = rhi::STOREACTION_STORE;
    visibilityConfig.priority = DAVA::PRIORITY_SERVICE_3D - 5;

    distanceMaterial->SetFXName(DAVA::FastName("~res:/LandscapeEditor/Materials/Distance.Opaque.material"));
    distanceMaterial->PreBuildMaterial(DAVA::PASS_FORWARD);

    prerenderMaterial->SetFXName(DAVA::FastName("~res:/LandscapeEditor/Materials/Distance.Prerender.material"));
    prerenderMaterial->PreBuildMaterial(DAVA::PASS_FORWARD);

    visibilityMaterial->SetFXName(DAVA::FastName("~res:/LandscapeEditor/Materials/CompareDistance.Opaque.material"));
    visibilityMaterial->AddFlag(DAVA::NMaterialFlagName::FLAG_BLENDING, DAVA::BLENDING_ADDITIVE);
    visibilityMaterial->PreBuildMaterial(DAVA::PASS_FORWARD);
}

VisibilityCheckRenderPass::~VisibilityCheckRenderPass()
{
}

void VisibilityCheckRenderPass::RenderToCubemapFromPoint(DAVA::RenderSystem* renderSystem, DAVA::Camera* fromCamera,
                                                         DAVA::Texture* renderTarget, const DAVA::Vector3& point)
{
    renderTargetConfig.colorBuffer[0].texture = renderTarget->handle;
    renderTargetConfig.depthStencilBuffer.texture = renderTarget->handleDepthStencil;
    renderTargetConfig.viewport.width = renderTarget->GetWidth();
    renderTargetConfig.viewport.height = renderTarget->GetHeight();

    camera->SetPosition(point);
    for (DAVA::uint32 i = 0; i < 6; ++i)
    {
        SetupCameraToRenderFromPointToFaceIndex(point, i);
        RenderWithCurrentSettings(renderSystem, fromCamera);
    }
}

void VisibilityCheckRenderPass::SetupCameraToRenderFromPointToFaceIndex(const DAVA::Vector3& point, DAVA::uint32 faceIndex)
{
    const DAVA::Vector3 directions[6] =
    {
      DAVA::Vector3(+1.0f, 0.0f, 0.0f),
      DAVA::Vector3(-1.0f, 0.0f, 0.0f),
      DAVA::Vector3(0.0f, +1.0f, 0.0f),
      DAVA::Vector3(0.0f, -1.0f, 0.0f),
      DAVA::Vector3(0.0f, 0.0f, +1.0f),
      DAVA::Vector3(0.0f, 0.0f, -1.0f),
    };
    const DAVA::Vector3 upVectors[6] =
    {
      DAVA::Vector3(0.0f, -1.0f, 0.0f),
      DAVA::Vector3(0.0f, -1.0f, 0.0f),
      DAVA::Vector3(0.0f, 0.0f, 1.0f),
      DAVA::Vector3(0.0f, 0.0f, -1.0f),
      DAVA::Vector3(0.0f, -1.0f, 0.0f),
      DAVA::Vector3(0.0f, -1.0f, 0.0f),
    };
    const rhi::TextureFace targetFaces[6] =
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

bool VisibilityCheckRenderPass::ShouldRenderObject(DAVA::RenderObject* object)
{
    auto type = object->GetType();

    return (type != DAVA::RenderObject::TYPE_SPEED_TREE) && (type != DAVA::RenderObject::TYPE_SPRITE) &&
    (type != DAVA::RenderObject::TYPE_VEGETATION) && (type != DAVA::RenderObject::TYPE_PARTICLE_EMTITTER);
}

bool VisibilityCheckRenderPass::ShouldRenderBatch(DAVA::RenderBatch* batch)
{
    return batch->GetMaterial()->GetEffectiveFXName() != DAVA::NMaterialName::SKYOBJECT;
}

void VisibilityCheckRenderPass::PreRenderScene(DAVA::RenderSystem* renderSystem, DAVA::Camera* fromCamera, DAVA::Texture* renderTarget)
{
    prerenderConfig.colorBuffer[0].texture = renderTarget->handle;
    prerenderConfig.depthStencilBuffer.texture = renderTarget->handleDepthStencil;

    rhi::HPacketList localPacketList;
    auto currentPass = rhi::AllocateRenderPass(prerenderConfig, 1, &localPacketList);
    rhi::BeginRenderPass(currentPass);
    rhi::BeginPacketList(localPacketList);

    // passConfig = prerenderConfig;
    DAVA::ShaderDescriptorCache::ClearDynamicBindigs();
    SetupCameraParams(fromCamera, fromCamera);
    PrepareVisibilityArrays(fromCamera, renderSystem);
    for (auto layer : renderLayers)
    {
        const DAVA::RenderBatchArray& renderBatchArray = layersBatchArrays[layer->GetRenderLayerID()];
        DAVA::uint32 batchCount = renderBatchArray.GetRenderBatchCount();
        for (DAVA::uint32 batchIndex = 0; batchIndex < batchCount; ++batchIndex)
        {
            DAVA::RenderBatch* batch = renderBatchArray.Get(batchIndex);
            DAVA::RenderObject* renderObject = batch->GetRenderObject();
            if (ShouldRenderBatch(batch) && ShouldRenderObject(renderObject))
            {
                renderObject->BindDynamicParameters(fromCamera);
                rhi::Packet packet;
                batch->BindGeometryData(packet);
                prerenderMaterial->BindParams(packet);
                rhi::AddPacket(localPacketList, packet);
            }
        }
    }

    rhi::EndPacketList(localPacketList);
    rhi::EndRenderPass(currentPass);
}

void VisibilityCheckRenderPass::RenderWithCurrentSettings(DAVA::RenderSystem* renderSystem, DAVA::Camera* sceneCamera)
{
    rhi::HPacketList localPacketList;
    auto renderTargetPass = rhi::AllocateRenderPass(renderTargetConfig, 1, &localPacketList);
    rhi::BeginRenderPass(renderTargetPass);
    rhi::BeginPacketList(localPacketList);

    // passConfig = renderTargetConfig;
    DAVA::ShaderDescriptorCache::ClearDynamicBindigs();
    SetupCameraParams(camera, camera);
    PrepareVisibilityArrays(camera, renderSystem);
    for (auto layer : renderLayers)
    {
        const DAVA::RenderBatchArray& renderBatchArray = layersBatchArrays[layer->GetRenderLayerID()];
        DAVA::uint32 batchCount = renderBatchArray.GetRenderBatchCount();
        for (DAVA::uint32 batchIndex = 0; batchIndex < batchCount; ++batchIndex)
        {
            DAVA::RenderBatch* batch = renderBatchArray.Get(batchIndex);
            DAVA::RenderObject* renderObject = batch->GetRenderObject();
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

void VisibilityCheckRenderPass::RenderVisibilityToTexture(DAVA::RenderSystem* renderSystem, DAVA::Camera* fromCamera, DAVA::Texture* cubemap,
                                                          DAVA::Texture* renderTarget, const DAVA::Vector3& point, const DAVA::Color& color)
{
    DAVA::FastName fnCubemap("cubemap");
    if (visibilityMaterial->HasLocalTexture(fnCubemap))
    {
        visibilityMaterial->SetTexture(fnCubemap, cubemap);
    }
    else
    {
        visibilityMaterial->AddTexture(fnCubemap, cubemap);
    }

    if (visibilityMaterial->HasLocalProperty(DAVA::NMaterialParamName::PARAM_FLAT_COLOR))
    {
        visibilityMaterial->SetPropertyValue(DAVA::NMaterialParamName::PARAM_FLAT_COLOR, color.color);
    }
    else
    {
        visibilityMaterial->AddProperty(DAVA::NMaterialParamName::PARAM_FLAT_COLOR, color.color, rhi::ShaderProp::TYPE_FLOAT4);
    }

    visibilityMaterial->PreBuildMaterial(DAVA::PASS_FORWARD);

    DAVA::Vector4 lightPosition(point.x, point.y, point.z, 0.0f);

    visibilityConfig.colorBuffer[0].texture = renderTarget->handle;
    visibilityConfig.depthStencilBuffer.texture = renderTarget->handleDepthStencil;

    rhi::HPacketList localPacketList;
    auto currentPass = rhi::AllocateRenderPass(visibilityConfig, 1, &localPacketList);
    rhi::BeginRenderPass(currentPass);
    rhi::BeginPacketList(localPacketList);

    DAVA::ShaderDescriptorCache::ClearDynamicBindigs();
    SetupCameraParams(fromCamera, fromCamera);
    PrepareVisibilityArrays(fromCamera, renderSystem);
    for (auto layer : renderLayers)
    {
        const DAVA::RenderBatchArray& renderBatchArray = layersBatchArrays[layer->GetRenderLayerID()];
        DAVA::uint32 batchCount = renderBatchArray.GetRenderBatchCount();
        for (DAVA::uint32 batchIndex = 0; batchIndex < batchCount; ++batchIndex)
        {
            DAVA::RenderBatch* batch = renderBatchArray.Get(batchIndex);
            DAVA::RenderObject* renderObject = batch->GetRenderObject();
            if (ShouldRenderBatch(batch) && ShouldRenderObject(renderObject))
            {
                renderObject->BindDynamicParameters(fromCamera);
                DAVA::Renderer::GetDynamicBindings().SetDynamicParam(DAVA::DynamicBindings::PARAM_LIGHT0_POSITION, &lightPosition,
                                                                     (DAVA::pointer_size)&lightPosition);

                rhi::Packet packet;
                batch->BindGeometryData(packet);
                visibilityMaterial->BindParams(packet);
                packet.depthStencilState = visibilityDepthStencilState;
                rhi::AddPacket(localPacketList, packet);
            }
        }
    }

    rhi::EndPacketList(localPacketList);
    rhi::EndRenderPass(currentPass);
}
