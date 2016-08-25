#include "VisibilityCheckRenderer.h"
#include "Render/ShaderCache.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"

const DAVA::FastName MaterialParamCubemap("cubemap");
const DAVA::FastName MaterialParamTransformedNormal("transformedNormal");
const DAVA::FastName MaterialParamPointProperties("pointProperties");
const DAVA::FastName MaterialParamOrigin("origin");
const DAVA::FastName MaterialParamFixedFrameMatrix("fixedFrameMatrix");
const DAVA::FastName MaterialParamCurrentFrameMatrix("currentFrameMatrix");
const DAVA::FastName MaterialParamCurrentFrameMatrixInverse("currentFrameMatrixInverse");
const DAVA::FastName MaterialParamFixedFrameTexture("fixedFrame");
const DAVA::FastName MaterialParamFixedFrameDistancesTexture("fixedFrameDistances");
const DAVA::FastName MaterialParamCurrentFrameTexture("currentFrame");
const DAVA::FastName MaterialParamViewportSize("viewportSize");
const DAVA::FastName MaterialParamCurrentFrameCompleteness("currentFrameCompleteness");

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

const DAVA::float32 VisibilityCheckRenderer::cameraNearClipPlane = 0.1f;

VisibilityCheckRenderer::VisibilityCheckRenderer()
    : cubemapCamera(new DAVA::Camera())
    , distanceMaterial(new DAVA::NMaterial())
    , visibilityMaterial(new DAVA::NMaterial())
    , prerenderMaterial(new DAVA::NMaterial())
    , reprojectionMaterial(new DAVA::NMaterial())
{
    cubemapCamera->SetupPerspective(90.0f, 1.0f, cameraNearClipPlane, 5000.0f);

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
    reprojectionMaterial->AddProperty(MaterialParamCurrentFrameCompleteness, &frameCompleteness, rhi::ShaderProp::Type::TYPE_FLOAT1);

    prerenderMaterial->SetFXName(DAVA::FastName("~res:/LandscapeEditor/Materials/Distance.Prerender.material"));

    distanceMaterial->SetFXName(DAVA::FastName("~res:/LandscapeEditor/Materials/Distance.Encode.material"));

    visibilityMaterial->SetFXName(DAVA::FastName("~res:/LandscapeEditor/Materials/Distance.Decode.material"));
    visibilityMaterial->AddFlag(DAVA::NMaterialFlagName::FLAG_BLENDING, DAVA::BLENDING_ADDITIVE);
    visibilityMaterial->AddProperty(DAVA::NMaterialParamName::PARAM_FLAT_COLOR, DAVA::Vector4().data, rhi::ShaderProp::TYPE_FLOAT4);
    visibilityMaterial->AddProperty(MaterialParamTransformedNormal, DAVA::Vector3().data, rhi::ShaderProp::TYPE_FLOAT3);
    visibilityMaterial->AddProperty(MaterialParamPointProperties, DAVA::Vector3().data, rhi::ShaderProp::TYPE_FLOAT3);
    visibilityMaterial->AddProperty(MaterialParamOrigin, DAVA::Vector3().data, rhi::ShaderProp::Type::TYPE_FLOAT3);
}

VisibilityCheckRenderer::~VisibilityCheckRenderer()
{
}

void VisibilityCheckRenderer::SetDelegate(VisibilityCheckRendererDelegate* de)
{
    renderDelegate = de;
}

void VisibilityCheckRenderer::RenderToCubemapFromPoint(DAVA::RenderSystem* renderSystem, const DAVA::Vector3& point, DAVA::Texture* cubemapTarget)
{
    renderTargetConfig.colorBuffer[0].targetTexture = cubemapTarget->handle;
    renderTargetConfig.depthStencilBuffer.targetTexture = cubemapTarget->handleDepthStencil;
    renderTargetConfig.viewport.width = cubemapTarget->GetWidth();
    renderTargetConfig.viewport.height = cubemapTarget->GetHeight();

    cubemapCamera->SetPosition(point);
    for (DAVA::uint32 i = 0; i < 6; ++i)
    {
        SetupCameraToRenderFromPointToFaceIndex(point, i);
        RenderWithCurrentSettings(renderSystem);
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

void VisibilityCheckRenderer::CollectRenderBatches(DAVA::RenderSystem* renderSystem, DAVA::Camera* fromCamera, DAVA::Vector<DAVA::RenderBatch*>& batches)
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
                            (material->GetRenderLayerID() == DAVA::RenderLayer::RENDER_LAYER_AFTER_OPAQUE_ID) ||
                            (material->GetRenderLayerID() == DAVA::RenderLayer::RENDER_LAYER_ALPHA_TEST_LAYER_ID))
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
    CollectRenderBatches(renderSystem, fromCamera, renderBatches);

    prerenderMaterial->PreBuildMaterial(DAVA::PASS_FORWARD);

    prerenderConfig.colorBuffer[0].targetTexture = renderTarget->handle;
    prerenderConfig.depthStencilBuffer.targetTexture = renderTarget->handleDepthStencil;
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

void VisibilityCheckRenderer::RenderWithCurrentSettings(DAVA::RenderSystem* renderSystem)
{
    DAVA::Vector<DAVA::RenderBatch*> renderBatches;
    CollectRenderBatches(renderSystem, cubemapCamera, renderBatches);

    distanceMaterial->PreBuildMaterial(DAVA::PASS_FORWARD);

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
    visibilityMaterial->SetPropertyValue(MaterialParamTransformedNormal, vp.normal.data);
    visibilityMaterial->SetPropertyValue(DAVA::NMaterialParamName::PARAM_FLAT_COLOR, vp.color.color);
    visibilityMaterial->PreBuildMaterial(DAVA::PASS_FORWARD);
}

void VisibilityCheckRenderer::RenderVisibilityToTexture(DAVA::RenderSystem* renderSystem, DAVA::Camera* batchesCamera,
                                                        DAVA::Camera* fromCamera, DAVA::Texture* cubemap, const VisbilityPoint& vp)
{
    DAVA::Vector<DAVA::RenderBatch*> renderBatches;
    UpdateVisibilityMaterialProperties(cubemap, vp);
    CollectRenderBatches(renderSystem, batchesCamera, renderBatches);

    DAVA::ShaderDescriptorCache::ClearDynamicBindigs();
    fromCamera->SetupDynamicParameters(false, nullptr);
    visibilityConfig.colorBuffer[0].targetTexture = renderTarget->handle;
    visibilityConfig.depthStencilBuffer.targetTexture = renderTarget->handleDepthStencil;

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

    visibilityMaterial->InvalidateRenderVariants();
    visibilityMaterial->InvalidateBufferBindings();

    prerenderMaterial->InvalidateRenderVariants();
    prerenderMaterial->InvalidateBufferBindings();

    reprojectionMaterial->InvalidateRenderVariants();
    reprojectionMaterial->InvalidateBufferBindings();
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
        renderTarget->SetMinMagFilter(rhi::TextureFilter::TEXFILTER_LINEAR, rhi::TextureFilter::TEXFILTER_LINEAR, rhi::TextureMipFilter::TEXMIPFILTER_NONE);
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
    DAVA::SafeRelease(fixedFrame);
    DAVA::SafeRelease(reprojectionTexture);
    DAVA::SafeRelease(distanceRenderTarget);

    auto rs2d = DAVA::RenderSystem2D::Instance();
    DAVA::float32 width = static_cast<DAVA::float32>(DAVA::Renderer::GetFramebufferWidth());
    DAVA::float32 height = static_cast<DAVA::float32>(DAVA::Renderer::GetFramebufferHeight());
    DAVA::uint32 w = renderTarget->GetWidth();
    DAVA::uint32 h = renderTarget->GetHeight();

    fixedFrame = DAVA::Texture::CreateFBO(w, h, DAVA::PixelFormat::FORMAT_RGBA8888, false, rhi::TextureType::TEXTURE_TYPE_2D, false);
    fixedFrame->SetMinMagFilter(rhi::TextureFilter::TEXFILTER_LINEAR, rhi::TextureFilter::TEXFILTER_LINEAR, rhi::TextureMipFilter::TEXMIPFILTER_NONE);
    fixedFrameMatrix = fromCamera->GetViewProjMatrix();
    fixedFrameCameraPosition = fromCamera->GetPosition();
    frameCompleteness = 1.0f;

    DAVA::RenderSystem2D::RenderTargetPassDescriptor desc;
    desc.clearColor = DAVA::Color::Clear;
    desc.colorAttachment = fixedFrame->handle;
    desc.depthAttachment = fixedFrame->handleDepthStencil;
    desc.transformVirtualToPhysical = false;
    rs2d->BeginRenderTargetPass(desc);
    rs2d->DrawTextureWithoutAdjustingRects(renderTarget, DAVA::RenderSystem2D::DEFAULT_2D_TEXTURE_ADDITIVE_MATERIAL, DAVA::Color::White,
                                           DAVA::Rect(0.0f, 0.0f, width, height), DAVA::Rect(0.0f, 0.0f, 1.0f, 1.0f));
    rs2d->EndRenderTargetPass();

    reprojectionTexture = DAVA::Texture::CreateFBO(w, h, DAVA::PixelFormat::FORMAT_RGBA8888, true, rhi::TextureType::TEXTURE_TYPE_2D, false);
    reprojectionConfig.colorBuffer[0].targetTexture = reprojectionTexture->handle;
    reprojectionConfig.depthStencilBuffer.targetTexture = reprojectionTexture->handleDepthStencil;

    distanceRenderTarget = DAVA::Texture::CreateFBO(w, h, TEXTURE_FORMAT, true, rhi::TEXTURE_TYPE_2D, false);
    distanceRenderTarget->SetMinMagFilter(rhi::TextureFilter::TEXFILTER_NEAREST, rhi::TextureFilter::TEXFILTER_NEAREST, rhi::TextureMipFilter::TEXMIPFILTER_NONE);
    distanceMapConfig.colorBuffer[0].targetTexture = distanceRenderTarget->handle;
    distanceMapConfig.depthStencilBuffer.targetTexture = distanceRenderTarget->handleDepthStencil;

    RenderToDistanceMapFromCamera(renderSystem, fromCamera);

    shouldFixFrame = false;
    frameFixed = true;
}

void VisibilityCheckRenderer::RenderToDistanceMapFromCamera(DAVA::RenderSystem* renderSystem, DAVA::Camera* fromCamera)
{
    DAVA::Vector<DAVA::RenderBatch*> renderBatches;
    CollectRenderBatches(renderSystem, fromCamera, renderBatches);

    distanceMaterial->PreBuildMaterial(DAVA::PASS_FORWARD);

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
    DAVA::Rect dstRect = DAVA::VirtualCoordinatesSystem::Instance()->ConvertPhysicalToVirtual(DAVA::Rect(0.0f, height, width, -height));

    if (frameFixed)
    {
        RenderWithReprojection(renderSystem, camera);
        rs2d->DrawTextureWithoutAdjustingRects(reprojectionTexture, DAVA::RenderSystem2D::DEFAULT_2D_TEXTURE_ADDITIVE_MATERIAL,
                                               DAVA::Color::White, dstRect, DAVA::Rect(0.0f, 0.0f, 1.0f, 1.0f));
    }
    else
    {
        rs2d->DrawTextureWithoutAdjustingRects(renderTarget, DAVA::RenderSystem2D::DEFAULT_2D_TEXTURE_ADDITIVE_MATERIAL,
                                               DAVA::Color::White, dstRect, DAVA::Rect(0.0f, 0.0f, 1.0f, 1.0f));
    }

    if (shouldFixFrame)
    {
        FixFrame(renderSystem, camera);
    }
}

void VisibilityCheckRenderer::RenderProgress(float ratio, const DAVA::Color& clr)
{
    frameCompleteness = ratio;
    auto rs2d = DAVA::RenderSystem2D::Instance();
    DAVA::float32 width = static_cast<DAVA::float32>(DAVA::Renderer::GetFramebufferWidth());
    rs2d->FillRect(DAVA::Rect(0.0f, 0.0f, frameCompleteness * width, 5.0f), clr);
}

void VisibilityCheckRenderer::RenderWithReprojection(DAVA::RenderSystem* renderSystem, DAVA::Camera* fromCamera)
{
    DAVA::Vector<DAVA::RenderBatch*> renderBatches;
    CollectRenderBatches(renderSystem, fromCamera, renderBatches);

    DAVA::Vector2 vpSize(static_cast<float>(reprojectionTexture->GetWidth()), static_cast<float>(reprojectionTexture->GetHeight()));
    reprojectionMaterial->SetPropertyValue(MaterialParamOrigin, fixedFrameCameraPosition.data);
    reprojectionMaterial->SetPropertyValue(MaterialParamFixedFrameMatrix, fixedFrameMatrix.data);
    reprojectionMaterial->SetPropertyValue(MaterialParamViewportSize, vpSize.data);
    reprojectionMaterial->SetPropertyValue(MaterialParamCurrentFrameCompleteness, &frameCompleteness);
    VCRLocal::PutTexture(reprojectionMaterial, MaterialParamFixedFrameTexture, fixedFrame);
    VCRLocal::PutTexture(reprojectionMaterial, MaterialParamFixedFrameDistancesTexture, distanceRenderTarget);
    VCRLocal::PutTexture(reprojectionMaterial, MaterialParamCurrentFrameTexture, renderTarget);
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
