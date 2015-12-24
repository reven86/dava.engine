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
#include "Render/2D/Systems/RenderSystem2D.h"

const DAVA::FastName MaterialParamCubemap("cubemap");
const DAVA::FastName MaterialParamTransformedNormal("transformedNormal");
const DAVA::FastName MaterialParamPointProperties("pointProperties");
const DAVA::FastName MaterialParamOrigin("origin");
const DAVA::FastName MaterialParamFixedFrameMatrix("fixedFrameMatrix");
const DAVA::FastName MaterialParamCurrentFrameMatrix("currentFrameMatrix");
const DAVA::FastName MaterialParamCurrentFrameMatrixInverse("currentFrameMatrixInverse");
const DAVA::FastName MaterialParamFixedFrameTexture("fixedFrame");
const DAVA::FastName MaterialParamFixedFrameDistancesTexture("fixedFrameDistances");
const DAVA::FastName MaterialParamViewportSize("viewportSize");

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
    , reprojectionMaterial(new DAVA::NMaterial())
{
    cubemapCamera->SetupPerspective(90.0f, 1.0f, 1.0f, 5000.0f);

    prerenderConfig.colorBuffer[0].loadAction = rhi::LOADACTION_CLEAR;
    prerenderConfig.depthStencilBuffer.loadAction = rhi::LOADACTION_CLEAR;
    prerenderConfig.depthStencilBuffer.storeAction = rhi::STOREACTION_STORE;
    prerenderConfig.priority = DAVA::PRIORITY_SERVICE_3D + 1;

    renderTargetConfig.colorBuffer[0].loadAction = rhi::LOADACTION_CLEAR;
    renderTargetConfig.depthStencilBuffer.loadAction = rhi::LOADACTION_CLEAR;
    renderTargetConfig.priority = DAVA::PRIORITY_SERVICE_3D;
    std::fill_n(renderTargetConfig.colorBuffer[0].clearColor, 4, 1.0f);

    rhi::DepthStencilState::Descriptor dsDesc;
    dsDesc.depthFunc = rhi::CMP_EQUAL;
    dsDesc.depthTestEnabled = 1;
    dsDesc.depthWriteEnabled = 0;
    visibilityDepthStencilState = rhi::AcquireDepthStencilState(dsDesc);

    dsDesc.depthFunc = rhi::CMP_LESS;
    dsDesc.depthTestEnabled = 1;
    dsDesc.depthWriteEnabled = 1;
    reprojectionDepthStencilState = rhi::AcquireDepthStencilState(dsDesc);

    visibilityConfig.colorBuffer[0].loadAction = rhi::LOADACTION_LOAD;
    visibilityConfig.depthStencilBuffer.loadAction = rhi::LOADACTION_LOAD;
    visibilityConfig.depthStencilBuffer.storeAction = rhi::STOREACTION_STORE;
    visibilityConfig.priority = DAVA::PRIORITY_SERVICE_3D - 1;

    reprojectionConfig.priority = DAVA::PRIORITY_SERVICE_3D - 2;
    reprojectionConfig.colorBuffer[0].clearColor[3] = 1.0f;

    distanceMapConfig.priority = DAVA::PRIORITY_SERVICE_3D - 1;
    distanceMapConfig.colorBuffer[0].clearColor[0] = 1.0f;
    distanceMapConfig.colorBuffer[0].clearColor[1] = 1.0f;
    distanceMapConfig.colorBuffer[0].clearColor[2] = 1.0f;
    distanceMapConfig.colorBuffer[0].clearColor[3] = 1.0f;
    distanceMapConfig.colorBuffer[0].loadAction = rhi::LOADACTION_CLEAR;
    distanceMapConfig.depthStencilBuffer.loadAction = rhi::LOADACTION_CLEAR;

    reprojectionMaterial->SetFXName(DAVA::FastName("~res:/LandscapeEditor/Materials/Distance.Reprojection.material"));
    reprojectionMaterial->AddProperty(MaterialParamCurrentFrameMatrix, DAVA::Matrix4::IDENTITY.data, rhi::ShaderProp::TYPE_FLOAT4X4);
    reprojectionMaterial->AddProperty(MaterialParamCurrentFrameMatrixInverse, DAVA::Matrix4::IDENTITY.data, rhi::ShaderProp::TYPE_FLOAT4X4);
    reprojectionMaterial->AddProperty(MaterialParamFixedFrameMatrix, DAVA::Matrix4::IDENTITY.data, rhi::ShaderProp::TYPE_FLOAT4X4);
    reprojectionMaterial->AddProperty(MaterialParamOrigin, DAVA::Vector3().data, rhi::ShaderProp::Type::TYPE_FLOAT3);
    reprojectionMaterial->AddProperty(MaterialParamViewportSize, DAVA::Vector2().data, rhi::ShaderProp::Type::TYPE_FLOAT2);
    reprojectionMaterial->PreBuildMaterial(DAVA::PASS_FORWARD);

    prerenderMaterial->SetFXName(DAVA::FastName("~res:/LandscapeEditor/Materials/Distance.Prerender.material"));
    prerenderMaterial->PreBuildMaterial(DAVA::PASS_FORWARD);

    distanceMaterial->SetFXName(DAVA::FastName("~res:/LandscapeEditor/Materials/Distance.Encode.material"));
    distanceMaterial->PreBuildMaterial(DAVA::PASS_FORWARD);

    visibilityMaterial->SetFXName(DAVA::FastName("~res:/LandscapeEditor/Materials/Distance.Decode.material"));
    visibilityMaterial->AddFlag(DAVA::NMaterialFlagName::FLAG_BLENDING, DAVA::BLENDING_ADDITIVE);
    visibilityMaterial->AddProperty(DAVA::NMaterialParamName::PARAM_FLAT_COLOR, DAVA::Vector4().data, rhi::ShaderProp::TYPE_FLOAT4);
    visibilityMaterial->AddProperty(MaterialParamTransformedNormal, DAVA::Vector3().data, rhi::ShaderProp::TYPE_FLOAT3);
    visibilityMaterial->AddProperty(MaterialParamPointProperties, DAVA::Vector3().data, rhi::ShaderProp::TYPE_FLOAT3);
    visibilityMaterial->AddProperty(MaterialParamOrigin, DAVA::Vector3().data, rhi::ShaderProp::Type::TYPE_FLOAT3);
    visibilityMaterial->PreBuildMaterial(DAVA::PASS_FORWARD);
}

VisibilityCheckRenderer::~VisibilityCheckRenderer()
{
}

void VisibilityCheckRenderer::SetDelegate(VisibilityCheckRendererDelegate* de)
{
    renderDelegate = de;
}

void VisibilityCheckRenderer::RenderToCubemapFromPoint(DAVA::RenderSystem* renderSystem, DAVA::Camera* fromCamera,
                                                       const DAVA::Vector3& point, DAVA::Texture* cubemapTarget)
{
    renderTargetConfig.colorBuffer[0].texture = cubemapTarget->handle;
    renderTargetConfig.depthStencilBuffer.texture = cubemapTarget->handleDepthStencil;
    renderTargetConfig.viewport.width = cubemapTarget->GetWidth();
    renderTargetConfig.viewport.height = cubemapTarget->GetHeight();

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
    renderTargetConfig.colorBuffer[0].textureFace = targetFaces[faceIndex];

    cubemapCamera->SetTarget(point + directions[faceIndex]);
    cubemapCamera->SetUp(upVectors[faceIndex]);
}

bool VisibilityCheckRenderer::ShouldRenderObject(DAVA::RenderObject* object)
{
    DVASSERT(renderDelegate != nullptr);
    return renderDelegate->ShouldDrawRenderObject(object);
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

    DAVA::Vector<DAVA::RenderObject*> renderObjects;
    renderSystem->GetRenderHierarchy()->Clip(fromCamera, renderObjects, DAVA::RenderObject::VISIBLE);

    for (auto renderObject : renderObjects)
    {
        if (ShouldRenderObject(renderObject))
        {
            if (renderObject->GetFlags() & DAVA::RenderObject::CUSTOM_PREPARE_TO_RENDER)
            {
                renderObject->PrepareToRender(fromCamera);
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

void VisibilityCheckRenderer::PreRenderScene(DAVA::RenderSystem* renderSystem, DAVA::Camera* fromCamera)
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

void VisibilityCheckRenderer::UpdateVisibilityMaterialProperties(DAVA::Texture* cubemapTexture, const VisbilityPoint& vp)
{
    if (visibilityMaterial->HasLocalTexture(MaterialParamCubemap))
    {
        visibilityMaterial->SetTexture(MaterialParamCubemap, cubemapTexture);
    }
    else
    {
        visibilityMaterial->AddTexture(MaterialParamCubemap, cubemapTexture);
    }

    DAVA::Vector3 propValue(vp.downAngleCosine, vp.upAngleCosine, vp.maxDistance);
    visibilityMaterial->SetPropertyValue(MaterialParamPointProperties, propValue.data);
    visibilityMaterial->SetPropertyValue(MaterialParamOrigin, vp.point.data);
    visibilityMaterial->SetPropertyValue(DAVA::NMaterialParamName::PARAM_FLAT_COLOR, vp.color.color);
    visibilityMaterial->PreBuildMaterial(DAVA::PASS_FORWARD);
}

void VisibilityCheckRenderer::RenderVisibilityToTexture(DAVA::RenderSystem* renderSystem, DAVA::Camera* fromCamera, DAVA::Texture* cubemap, const VisbilityPoint& vp)
{
    DAVA::Vector<DAVA::RenderBatch*> renderBatches;
    UpdateVisibilityMaterialProperties(cubemap, vp);
    CollectRenderBatches(renderSystem, fromCamera, fromCamera, renderBatches);

    visibilityConfig.colorBuffer[0].texture = renderTarget->handle;
    visibilityConfig.depthStencilBuffer.texture = renderTarget->handleDepthStencil;
    RenderPassScope pass(visibilityConfig);
    for (auto batch : renderBatches)
    {
        batch->GetRenderObject()->BindDynamicParameters(fromCamera);
        rhi::Packet packet;
        batch->BindGeometryData(packet);
        visibilityMaterial->BindParams(packet);
        packet.depthStencilState = visibilityDepthStencilState;
        rhi::AddPacket(pass.packetList, packet);
    }
}

void VisibilityCheckRenderer::InvalidateMaterials()
{
    distanceMaterial->InvalidateRenderVariants();
    distanceMaterial->InvalidateBufferBindings();
    distanceMaterial->PreBuildMaterial(DAVA::PASS_FORWARD);

    visibilityMaterial->InvalidateRenderVariants();
    visibilityMaterial->InvalidateBufferBindings();
    visibilityMaterial->PreBuildMaterial(DAVA::PASS_FORWARD);

    prerenderMaterial->InvalidateRenderVariants();
    prerenderMaterial->InvalidateBufferBindings();
    prerenderMaterial->PreBuildMaterial(DAVA::PASS_FORWARD);

    reprojectionMaterial->InvalidateRenderVariants();
    reprojectionMaterial->InvalidateBufferBindings();
    reprojectionMaterial->PreBuildMaterial(DAVA::PASS_FORWARD);
}

void VisibilityCheckRenderer::FixFrame()
{
    shouldFixFrame = true;
}

void VisibilityCheckRenderer::ReleaseFrame()
{
    frameFixed = false;
    shouldFixFrame = false;
}

void VisibilityCheckRenderer::CreateOrUpdateRenderTarget(const DAVA::Size2i& sz)
{
    if ((renderTarget == nullptr) || (renderTarget->GetWidth() != sz.dx) || (renderTarget->GetHeight() != sz.dy))
    {
        SafeRelease(renderTarget);
        renderTarget = DAVA::Texture::CreateFBO(sz.dx, sz.dy, DAVA::PixelFormat::FORMAT_RGBA8888, true, rhi::TEXTURE_TYPE_2D, false);
    }
}

namespace VCRLocal
{
inline void PutTexture(DAVA::NMaterial* mat, const DAVA::FastName& slot, DAVA::Texture* tex)
{
    if (mat->HasLocalTexture(slot))
    {
        mat->SetTexture(slot, tex);
    }
    else
    {
        mat->AddTexture(slot, tex);
    }
}
}

void VisibilityCheckRenderer::FixFrame(DAVA::RenderSystem* renderSystem, DAVA::Camera* fromCamera)
{
    auto rs2d = DAVA::RenderSystem2D::Instance();
    DAVA::float32 width = static_cast<DAVA::float32>(DAVA::Renderer::GetFramebufferWidth());
    DAVA::float32 height = static_cast<DAVA::float32>(DAVA::Renderer::GetFramebufferHeight());

    DAVA::SafeRelease(fixedFrame);
    DAVA::SafeRelease(reprojectionTexture);
    SafeRelease(distanceRenderTarget);

    DAVA::uint32 w = renderTarget->GetWidth();
    DAVA::uint32 h = renderTarget->GetHeight();

    fixedFrame = DAVA::Texture::CreateFBO(w, h, DAVA::PixelFormat::FORMAT_RGBA8888, false, rhi::TextureType::TEXTURE_TYPE_2D, false);
    fixedFrameMatrix = fromCamera->GetViewProjMatrix();
    fixedFrameCameraPosition = fromCamera->GetPosition();

    DAVA::Rect dstRect = DAVA::Rect(0.0f, 0.0f, width, height);
    DAVA::RenderSystem2D::RenderTargetPassDescriptor desc;
    desc.clearColor = DAVA::Color::Clear;
    desc.target = fixedFrame;
    desc.shouldClear = true;
    desc.shouldTransformVirtualToPhysical = false;
    rs2d->BeginRenderTargetPass(desc);
    rs2d->DrawTexture(renderTarget, DAVA::RenderSystem2D::DEFAULT_2D_TEXTURE_ADDITIVE_MATERIAL, DAVA::Color::White, dstRect);
    rs2d->EndRenderTargetPass();

    reprojectionTexture = DAVA::Texture::CreateFBO(w, h, DAVA::PixelFormat::FORMAT_RGBA8888, true, rhi::TextureType::TEXTURE_TYPE_2D, false);
    reprojectionConfig.colorBuffer[0].texture = reprojectionTexture->handle;
    reprojectionConfig.depthStencilBuffer.texture = reprojectionTexture->handleDepthStencil;

    distanceRenderTarget = DAVA::Texture::CreateFBO(w, h, DAVA::PixelFormat::FORMAT_RGBA8888, true, rhi::TEXTURE_TYPE_2D, false);
    distanceMapConfig.colorBuffer[0].texture = distanceRenderTarget->handle;
    distanceMapConfig.depthStencilBuffer.texture = distanceRenderTarget->handleDepthStencil;
    RenderToDistanceMapFromCamera(renderSystem, fromCamera);

    VCRLocal::PutTexture(reprojectionMaterial, MaterialParamFixedFrameTexture, fixedFrame);
    VCRLocal::PutTexture(reprojectionMaterial, MaterialParamFixedFrameDistancesTexture, distanceRenderTarget);

    shouldFixFrame = false;
    frameFixed = true;
}

void VisibilityCheckRenderer::RenderToDistanceMapFromCamera(DAVA::RenderSystem* renderSystem, DAVA::Camera* fromCamera)
{
    DAVA::Vector<DAVA::RenderBatch*> renderBatches;
    CollectRenderBatches(renderSystem, fromCamera, fromCamera, renderBatches);

    RenderPassScope pass(distanceMapConfig);
    for (auto batch : renderBatches)
    {
        rhi::Packet packet;
        batch->GetRenderObject()->BindDynamicParameters(fromCamera);
        batch->BindGeometryData(packet);
        distanceMaterial->BindParams(packet);
        rhi::AddPacket(pass.packetList, packet);
    }
}

void VisibilityCheckRenderer::RenderCurrentOverlayTexture(DAVA::RenderSystem* renderSystem, DAVA::Camera* camera)
{
    auto rs2d = DAVA::RenderSystem2D::Instance();
    DAVA::float32 width = static_cast<DAVA::float32>(DAVA::Renderer::GetFramebufferWidth());
    DAVA::float32 height = static_cast<DAVA::float32>(DAVA::Renderer::GetFramebufferHeight());
    DAVA::Rect dstRect(0.0f, height, width, -height);

    const DAVA::Matrix4& currentMatrix = camera->GetViewProjMatrix();
    if (frameFixed)
    {
        RenderWithReprojection(renderSystem, camera);
        rs2d->DrawTextureWithoutAdjustingRects(reprojectionTexture, DAVA::RenderSystem2D::DEFAULT_2D_TEXTURE_ADDITIVE_MATERIAL, DAVA::Color::White, dstRect);
    }
    else
    {
        rs2d->DrawTextureWithoutAdjustingRects(renderTarget, DAVA::RenderSystem2D::DEFAULT_2D_TEXTURE_ADDITIVE_MATERIAL, DAVA::Color::White, dstRect);
    }

    if (shouldFixFrame)
    {
        FixFrame(renderSystem, camera);
    }
}

void VisibilityCheckRenderer::RenderProgress(float ratio)
{
    auto rs2d = DAVA::RenderSystem2D::Instance();
    DAVA::float32 width = static_cast<DAVA::float32>(DAVA::Renderer::GetFramebufferWidth());
    rs2d->FillRect(DAVA::Rect(0.0f, 0.0f, ratio * width, 5.0f), DAVA::Color::White);
}

void VisibilityCheckRenderer::RenderWithReprojection(DAVA::RenderSystem* renderSystem, DAVA::Camera* fromCamera)
{
    DAVA::Vector<DAVA::RenderBatch*> renderBatches;
    CollectRenderBatches(renderSystem, fromCamera, fromCamera, renderBatches);

    reprojectionMaterial->SetPropertyValue(MaterialParamOrigin, fixedFrameCameraPosition.data);
    reprojectionMaterial->SetPropertyValue(MaterialParamFixedFrameMatrix, fixedFrameMatrix.data);
    reprojectionMaterial->SetPropertyValue(MaterialParamViewportSize, DAVA::Vector2(static_cast<float>(reprojectionTexture->GetWidth()),
                                                                                    static_cast<float>(reprojectionTexture->GetHeight()))
                                                                      .data);
    reprojectionMaterial->PreBuildMaterial(DAVA::PASS_FORWARD);

    RenderPassScope pass(reprojectionConfig);
    for (auto batch : renderBatches)
    {
        rhi::Packet packet;
        batch->GetRenderObject()->BindDynamicParameters(fromCamera);
        batch->BindGeometryData(packet);
        reprojectionMaterial->BindParams(packet);
        packet.depthStencilState = reprojectionDepthStencilState;
        rhi::AddPacket(pass.packetList, packet);
    }
}
