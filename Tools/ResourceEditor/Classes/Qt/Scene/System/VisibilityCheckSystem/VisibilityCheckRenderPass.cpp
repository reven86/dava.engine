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

using namespace DAVA;

const uint32 cubemapFaces = 6;

VisibilityCheckRenderPass::VisibilityCheckRenderPass()
    : RenderPass(PASS_FORWARD)
    , camera(new Camera())
    , distanceMaterial(new NMaterial())
    , visibilityMaterial(new NMaterial())
    , prerenderMaterial(new NMaterial())
{
    camera->SetupPerspective(90.0f, 1.0f, 1.0f, 5000.0f);

    auto sortingFlags = RenderBatchArray::SORT_THIS_FRAME | RenderBatchArray::SORT_BY_DISTANCE_BACK_TO_FRONT;
    AddRenderLayer(new RenderLayer(RenderLayer::RENDER_LAYER_OPAQUE_ID, sortingFlags));
    AddRenderLayer(new RenderLayer(RenderLayer::RENDER_LAYER_AFTER_OPAQUE_ID, sortingFlags));

    prerenderConfig.colorBuffer[0].loadAction = rhi::LOADACTION_CLEAR;
    prerenderConfig.depthStencilBuffer.loadAction = rhi::LOADACTION_CLEAR;
    prerenderConfig.depthStencilBuffer.storeAction = rhi::STOREACTION_STORE;
    prerenderConfig.priority = PRIORITY_SERVICE_3D + 5;

    renderTargetConfig.colorBuffer[0].loadAction = rhi::LOADACTION_CLEAR;
    renderTargetConfig.depthStencilBuffer.loadAction = rhi::LOADACTION_CLEAR;
    renderTargetConfig.priority = PRIORITY_SERVICE_3D;
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
    visibilityConfig.priority = PRIORITY_SERVICE_3D - 5;

    distanceMaterial->SetFXName(FastName("~res:/LandscapeEditor/Materials/Distance.Opaque.material"));
    distanceMaterial->PreBuildMaterial(PASS_FORWARD);

    prerenderMaterial->SetFXName(FastName("~res:/LandscapeEditor/Materials/Distance.Prerender.material"));
    prerenderMaterial->PreBuildMaterial(PASS_FORWARD);

    visibilityMaterial->SetFXName(FastName("~res:/LandscapeEditor/Materials/CompareDistance.Opaque.material"));
    visibilityMaterial->AddFlag(NMaterialFlagName::FLAG_BLENDING, BLENDING_ADDITIVE);
    visibilityMaterial->PreBuildMaterial(PASS_FORWARD);
}

VisibilityCheckRenderPass::~VisibilityCheckRenderPass()
{
}

void VisibilityCheckRenderPass::RenderToCubemapFromPoint(RenderSystem* renderSystem, Camera* fromCamera, Texture* renderTarget, const Vector3& point)
{
    renderTargetConfig.colorBuffer[0].texture = renderTarget->handle;
    renderTargetConfig.depthStencilBuffer.texture = renderTarget->handleDepthStencil;
    renderTargetConfig.viewport.width = renderTarget->GetWidth();
    renderTargetConfig.viewport.height = renderTarget->GetHeight();

    camera->SetPosition(point);
    for (uint32 i = 0; i < cubemapFaces; ++i)
    {
        SetupCameraToRenderFromPointToFaceIndex(point, i);
        RenderWithCurrentSettings(renderSystem, fromCamera);
    }
}

void VisibilityCheckRenderPass::SetupCameraToRenderFromPointToFaceIndex(const Vector3& point, uint32 faceIndex)
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

bool VisibilityCheckRenderPass::ShouldRenderObject(RenderObject* object)
{
    auto type = object->GetType();

    return (type != RenderObject::TYPE_SPEED_TREE) && (type != RenderObject::TYPE_SPRITE) &&
    (type != RenderObject::TYPE_VEGETATION) && (type != RenderObject::TYPE_PARTICLE_EMTITTER);
}

bool VisibilityCheckRenderPass::ShouldRenderBatch(RenderBatch* batch)
{
    return (batch->GetMaterial()->GetEffectiveFXName() != NMaterialName::SKYOBJECT);
}

void VisibilityCheckRenderPass::PreRenderScene(RenderSystem* renderSystem, Camera* fromCamera, Texture* renderTarget)
{
    prerenderConfig.colorBuffer[0].texture = renderTarget->handle;
    prerenderConfig.depthStencilBuffer.texture = renderTarget->handleDepthStencil;

    rhi::HPacketList localPacketList;
    auto currentPass = rhi::AllocateRenderPass(prerenderConfig, 1, &localPacketList);
    rhi::BeginRenderPass(currentPass);
    rhi::BeginPacketList(localPacketList);

    // passConfig = prerenderConfig;
    ShaderDescriptorCache::ClearDynamicBindigs();
    SetupCameraParams(fromCamera, fromCamera);
    PrepareVisibilityArrays(fromCamera, renderSystem);
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

void VisibilityCheckRenderPass::RenderWithCurrentSettings(RenderSystem* renderSystem, Camera* sceneCamera)
{
    rhi::HPacketList localPacketList;
    auto renderTargetPass = rhi::AllocateRenderPass(renderTargetConfig, 1, &localPacketList);
    rhi::BeginRenderPass(renderTargetPass);
    rhi::BeginPacketList(localPacketList);

    // passConfig = renderTargetConfig;
    ShaderDescriptorCache::ClearDynamicBindigs();
    SetupCameraParams(camera, camera);
    PrepareVisibilityArrays(camera, renderSystem);
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

void VisibilityCheckRenderPass::RenderVisibilityToTexture(RenderSystem* renderSystem, Camera* fromCamera, Texture* cubemap,
                                                          Texture* renderTarget, const Vector3& point, const Color& color)
{
    FastName fnCubemap("cubemap");
    if (visibilityMaterial->HasLocalTexture(fnCubemap))
    {
        visibilityMaterial->SetTexture(fnCubemap, cubemap);
    }
    else
    {
        visibilityMaterial->AddTexture(fnCubemap, cubemap);
    }

    if (visibilityMaterial->HasLocalProperty(NMaterialParamName::PARAM_FLAT_COLOR))
    {
        visibilityMaterial->SetPropertyValue(NMaterialParamName::PARAM_FLAT_COLOR, color.color);
    }
    else
    {
        visibilityMaterial->AddProperty(NMaterialParamName::PARAM_FLAT_COLOR, color.color, rhi::ShaderProp::TYPE_FLOAT4);
    }

    visibilityMaterial->PreBuildMaterial(PASS_FORWARD);

    Vector4 lightPosition(point.x, point.y, point.z, 0.0f);

    visibilityConfig.colorBuffer[0].texture = renderTarget->handle;
    visibilityConfig.depthStencilBuffer.texture = renderTarget->handleDepthStencil;

    rhi::HPacketList localPacketList;
    auto currentPass = rhi::AllocateRenderPass(visibilityConfig, 1, &localPacketList);
    rhi::BeginRenderPass(currentPass);
    rhi::BeginPacketList(localPacketList);

    // passConfig = visibilityConfig;
    ShaderDescriptorCache::ClearDynamicBindigs();
    SetupCameraParams(fromCamera, fromCamera);
    PrepareVisibilityArrays(fromCamera, renderSystem);
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
                renderObject->BindDynamicParameters(fromCamera);
                Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_LIGHT0_POSITION, &lightPosition, (pointer_size)&lightPosition);

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
