#include "Classes/Qt/Scene/System/ParticleEffectDebugDrawSystem/ParticleDebugDrawQuadRenderPass.h"
#include "Render/RHI/rhi_Type.h"
#include "Render/RHI/rhi_Public.h"


using namespace DAVA;

const FastName ParticleDebugDrawQuadRenderPass::PASS_DEBUG_DRAW_QUAD("DebugDrawQuad");
const Array<ParticleDebugDrawQuadRenderPass::VertexPT, 6> ParticleDebugDrawQuadRenderPass::quad =
{ {
    { Vector3(-10.f, 0.f, -10.f), Vector2(0.0f, 1.0f) },
    { Vector3(10.f, 0.f, -10.f), Vector2(1.0f, 1.0f) },
    { Vector3(-10.f, 0.f, 10.f), Vector2(0.0f, 0.0f) },
    { Vector3(-10.f, 0.f, 10.f), Vector2(0.0f, 0.0f) },
    { Vector3(10.f, 0.f, -10.f), Vector2(1.0f, 1.0f) },
    { Vector3(10.f, 0.f, 10.f), Vector2(1.0f, 0.0f) }
} };


void ParticleDebugDrawQuadRenderPass::PrepareRenderData()
{
    rhi::VertexBuffer::Descriptor vDesc = {};
    vDesc.size = sizeof(VertexPT) * 6;
    vDesc.initialData = quad.data();
    vDesc.usage = rhi::USAGE_STATICDRAW;
    quadBuffer = rhi::CreateVertexBuffer(vDesc);

    rhi::VertexLayout layout;
    layout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);
    layout.AddElement(rhi::VS_TEXCOORD, 0, rhi::VDT_FLOAT, 2);

    quadPacket.vertexLayoutUID = rhi::VertexLayout::UniqueId(layout);
    quadPacket.vertexStreamCount = 1;
    quadPacket.vertexStream[0] = quadBuffer;
    
    quadPacket.primitiveType = rhi::PRIMITIVE_TRIANGLELIST;
    quadPacket.primitiveCount = 2;
    
    quadMaterial = new NMaterial();
    quadMaterial->SetFXName(NMaterialName::TEXTURED_OPAQUE);
    quadMaterial->AddTexture(NMaterialTextureName::TEXTURE_ALBEDO, debugTexture);
    quadMaterial->PreBuildMaterial(PASS_FORWARD);
}

void ParticleDebugDrawQuadRenderPass::ByndDynamicParams(Camera* cam)
{
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_LIGHT0_POSITION, &Vector4::Zero, reinterpret_cast<pointer_size>(&Vector4::Zero));
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_LIGHT0_COLOR, &Color::Black, reinterpret_cast<pointer_size>(&Color::Black));
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_LIGHT0_AMBIENT_COLOR, &Color::Black, reinterpret_cast<pointer_size>(&Color::Black));
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_WORLD, &Matrix4::IDENTITY, reinterpret_cast<pointer_size>(&Matrix4::IDENTITY));
    AABBox3 bbox(Vector3(-100, -100, -100), Vector3(100, 100, 100));
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_LOCAL_BOUNDING_BOX, &bbox, reinterpret_cast<pointer_size>(&bbox));
}

ParticleDebugDrawQuadRenderPass::ParticleDebugDrawQuadRenderPass(const DAVA::FastName& name, DAVA::RenderSystem* renderSystem, DAVA::Texture* texture) 
    : RenderPass(name), debugTexture(texture)
{
    passConfig = renderSystem->GetMainPassConfig();
//     passConfig.priority = DAVA::PRIORITY_MAIN_3D + 2;
    
    SetRenderTargetProperties(passConfig.viewport.width, passConfig.viewport.height, DAVA::PixelFormat::FORMAT_RGBA8888);

    PrepareRenderData();
}

ParticleDebugDrawQuadRenderPass::~ParticleDebugDrawQuadRenderPass()
{
    rhi::DeleteVertexBuffer(quadBuffer);
    SafeRelease(quadMaterial);
}

void ParticleDebugDrawQuadRenderPass::Draw(DAVA::RenderSystem* renderSystem)
{
    Camera* cam = renderSystem->GetDrawCamera();
    SetupCameraParams(cam, cam);

    if (BeginRenderPass())
    {        
        ByndDynamicParams(cam);
        quadMaterial->BindParams(quadPacket);
        //quadPacket.cullMode = rhi::CULL_NONE;
        rhi::AddPacket(packetList, quadPacket);        
        EndRenderPass();        
    }
}
