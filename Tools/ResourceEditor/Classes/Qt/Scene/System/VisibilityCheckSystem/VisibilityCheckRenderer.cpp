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

#include "VisibilityCheckRenderer.h"
#include "Render/ShaderCache.h"

struct RenderPassScope
{
    rhi::HRenderPass renderPass;
    rhi::HPacketList packetList;

    RenderPassScope(const rhi::RenderPassConfig& config)
    {
        renderPass = rhi::AllocateRenderPass(config, 1, &packetList);
        rhi::BeginRenderPass(renderPass);
        rhi::BeginPacketList(packetList);
    }

    ~RenderPassScope()
    {
        rhi::EndPacketList(packetList);
        rhi::EndRenderPass(renderPass);
    }
};

VisibilityCheckRenderer::VisibilityCheckRenderer()
    : cubemapCamera(new DAVA::Camera())
    , distanceMaterial(new DAVA::NMaterial())
    , visibilityMaterial(new DAVA::NMaterial())
    , prerenderMaterial(new DAVA::NMaterial())
{
    cubemapCamera->SetupPerspective(90.0f, 1.0f, 1.0f, 5000.0f);

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

VisibilityCheckRenderer::~VisibilityCheckRenderer()
{
}

void VisibilityCheckRenderer::RenderToCubemapFromPoint(DAVA::RenderSystem* renderSystem, DAVA::Camera* fromCamera,
                                                       DAVA::Texture* renderTarget, const DAVA::Vector3& point)
{
    renderTargetConfig.colorBuffer[0].texture = renderTarget->handle;
    renderTargetConfig.depthStencilBuffer.texture = renderTarget->handleDepthStencil;
    renderTargetConfig.viewport.width = renderTarget->GetWidth();
    renderTargetConfig.viewport.height = renderTarget->GetHeight();

    cubemapCamera->SetPosition(point);
    for (DAVA::uint32 i = 0; i < 6; ++i)
    {
        SetupCameraToRenderFromPointToFaceIndex(point, i);
        RenderWithCurrentSettings(renderSystem, fromCamera);
    }
}

void VisibilityCheckRenderer::SetupCameraToRenderFromPointToFaceIndex(const DAVA::Vector3& point, DAVA::uint32 faceIndex)
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

    cubemapCamera->SetTarget(point + directions[faceIndex]);
    cubemapCamera->SetUp(upVectors[faceIndex]);
}

bool VisibilityCheckRenderer::ShouldRenderObject(DAVA::RenderObject* object)
{
    auto type = object->GetType();

    return (type != DAVA::RenderObject::TYPE_SPEED_TREE) && (type != DAVA::RenderObject::TYPE_SPRITE) &&
    (type != DAVA::RenderObject::TYPE_VEGETATION) && (type != DAVA::RenderObject::TYPE_PARTICLE_EMTITTER);
}

bool VisibilityCheckRenderer::ShouldRenderBatch(DAVA::RenderBatch* batch)
{
    return batch->GetMaterial()->GetEffectiveFXName() != DAVA::NMaterialName::SKYOBJECT;
}

void VisibilityCheckRenderer::CollectRenderBatches(DAVA::RenderSystem* renderSystem, DAVA::Camera* fromCamera,
                                                   DAVA::Camera* lodCamera, DAVA::Vector<DAVA::RenderBatch*>& batches)
{
    DAVA::ShaderDescriptorCache::ClearDynamicBindigs();
    fromCamera->SetupDynamicParameters(false, nullptr);

    DAVA::Vector<DAVA::RenderObject*> rendeObjects;
    renderSystem->GetRenderHierarchy()->Clip(fromCamera, rendeObjects, DAVA::RenderObject::VISIBLE);

    for (auto renderObject : rendeObjects)
    {
        if (ShouldRenderObject(renderObject))
        {
            if (renderObject->GetFlags() & DAVA::RenderObject::CUSTOM_PREPARE_TO_RENDER)
            {
                renderObject->PrepareToRender(lodCamera);
            }

            DAVA::uint32 batchCount = renderObject->GetActiveRenderBatchCount();
            for (DAVA::uint32 batchIndex = 0; batchIndex < batchCount; ++batchIndex)
            {
                DAVA::RenderBatch* batch = renderObject->GetActiveRenderBatch(batchIndex);
                if (ShouldRenderBatch(batch))
                {
                    DAVA::NMaterial* material = batch->GetMaterial();
                    if ((material != nullptr) && material->PreBuildMaterial(DAVA::PASS_FORWARD))
                    {
                        if ((material->GetRenderLayerID() == DAVA::RenderLayer::RENDER_LAYER_OPAQUE_ID) ||
                            (material->GetRenderLayerID() == DAVA::RenderLayer::RENDER_LAYER_AFTER_OPAQUE_ID))
                        {
                            batches.push_back(batch);
                        }
                    }
                }
            }
        }
    }
}

void VisibilityCheckRenderer::PreRenderScene(DAVA::RenderSystem* renderSystem, DAVA::Camera* fromCamera, DAVA::Texture* renderTarget)
{
    DAVA::Vector<DAVA::RenderBatch*> renderBatches;
    CollectRenderBatches(renderSystem, fromCamera, fromCamera, renderBatches);

    prerenderConfig.colorBuffer[0].texture = renderTarget->handle;
    prerenderConfig.depthStencilBuffer.texture = renderTarget->handleDepthStencil;
    RenderPassScope pass(prerenderConfig);
    for (auto batch : renderBatches)
    {
        batch->GetRenderObject()->BindDynamicParameters(fromCamera);
        rhi::Packet packet;
        batch->BindGeometryData(packet);
        prerenderMaterial->BindParams(packet);
        rhi::AddPacket(pass.packetList, packet);
    }
}

void VisibilityCheckRenderer::RenderWithCurrentSettings(DAVA::RenderSystem* renderSystem, DAVA::Camera* sceneCamera)
{
    DAVA::Vector<DAVA::RenderBatch*> renderBatches;
    CollectRenderBatches(renderSystem, cubemapCamera, sceneCamera, renderBatches);

    RenderPassScope pass(renderTargetConfig);
    for (auto batch : renderBatches)
    {
        batch->GetRenderObject()->BindDynamicParameters(cubemapCamera);
        rhi::Packet packet;
        batch->BindGeometryData(packet);
        distanceMaterial->BindParams(packet);
        packet.cullMode = rhi::CULL_NONE;
        rhi::AddPacket(pass.packetList, packet);
    }
}

void VisibilityCheckRenderer::UpdateVisibilityMaterialProperties(DAVA::Texture* cubemapTexture, const DAVA::Color& color)
{
    DAVA::FastName fnCubemap("cubemap");
    if (visibilityMaterial->HasLocalTexture(fnCubemap))
    {
        visibilityMaterial->SetTexture(fnCubemap, cubemapTexture);
    }
    else
    {
        visibilityMaterial->AddTexture(fnCubemap, cubemapTexture);
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
}

void VisibilityCheckRenderer::RenderVisibilityToTexture(DAVA::RenderSystem* renderSystem, DAVA::Camera* fromCamera, DAVA::Texture* cubemap,
                                                        DAVA::Texture* renderTarget, const DAVA::Vector3& point, const DAVA::Color& color)
{
    DAVA::Vector<DAVA::RenderBatch*> renderBatches;
    UpdateVisibilityMaterialProperties(cubemap, color);
    CollectRenderBatches(renderSystem, fromCamera, fromCamera, renderBatches);

    DAVA::Vector4 pointLocation(point.x, point.y, point.z, 0.0f);

    visibilityConfig.colorBuffer[0].texture = renderTarget->handle;
    visibilityConfig.depthStencilBuffer.texture = renderTarget->handleDepthStencil;
    RenderPassScope pass(visibilityConfig);
    for (auto batch : renderBatches)
    {
        batch->GetRenderObject()->BindDynamicParameters(fromCamera);
        DAVA::Renderer::GetDynamicBindings().SetDynamicParam(DAVA::DynamicBindings::PARAM_LIGHT0_POSITION, &pointLocation,
                                                             reinterpret_cast<DAVA::pointer_size>(&pointLocation));
        rhi::Packet packet;
        batch->BindGeometryData(packet);
        visibilityMaterial->BindParams(packet);
        packet.depthStencilState = visibilityDepthStencilState;
        rhi::AddPacket(pass.packetList, packet);
    }
}
