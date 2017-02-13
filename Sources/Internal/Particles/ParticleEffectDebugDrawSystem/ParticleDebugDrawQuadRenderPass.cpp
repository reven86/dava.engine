#include "Particles/ParticleEffectDebugDrawSystem/ParticleDebugDrawQuadRenderPass.h"

#include "Particles/ParticleEffectDebugDrawSystem/ParticleEffectDebugDrawSystem.h"

#include "Render/RHI/rhi_Type.h"
#include "Render/RHI/rhi_Public.h"

namespace DAVA
{
const FastName ParticleDebugDrawQuadRenderPass::PASS_DEBUG_DRAW_QUAD("ForwardPass");

void ParticleDebugDrawQuadRenderPass::PrepareRenderData()
{
    static const int vertsCount = 6;
    static const Array<ParticleDebugDrawQuadRenderPass::VertexPT, vertsCount> quad =
    { {
    { Vector3(-1.f, 0.f, -1.f), Vector2(0.0f, 1.0f) },
    { Vector3(1.f, 0.f, -1.f), Vector2(1.0f, 1.0f) },
    { Vector3(-1.f, 0.f, 1.f), Vector2(0.0f, 0.0f) },
    { Vector3(-1.f, 0.f, 1.f), Vector2(0.0f, 0.0f) },
    { Vector3(1.f, 0.f, -1.f), Vector2(1.0f, 1.0f) },
    { Vector3(1.f, 0.f, 1.f), Vector2(1.0f, 0.0f) }
    } };

    rhi::VertexBuffer::Descriptor vDesc = {};
    vDesc.size = sizeof(VertexPT) * vertsCount;
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

ParticleDebugDrawQuadRenderPass::ParticleDebugDrawQuadRenderPass(ParticleDebugQuadRenderPassConfig config)
    : RenderPass(config.name)
    , quadMaterial(config.quadMaterial)
    , quadHeatMaterial(config.quadHeatMaterial)
    , drawMode(config.drawMode)
{
    passConfig = config.renderSystem->GetMainPassConfig();
    passConfig.colorBuffer[0].loadAction = rhi::LOADACTION_LOAD;

    quadMaterial->PreBuildMaterial(passName);
    quadHeatMaterial->PreBuildMaterial(passName);

    SetRenderTargetProperties(passConfig.viewport.width, passConfig.viewport.height, DAVA::PixelFormat::FORMAT_RGBA8888);
    PrepareRenderData();
}

ParticleDebugDrawQuadRenderPass::~ParticleDebugDrawQuadRenderPass()
{
    rhi::DeleteVertexBuffer(quadBuffer);
}

void ParticleDebugDrawQuadRenderPass::Draw(DAVA::RenderSystem* renderSystem)
{
    Camera* cam = renderSystem->GetDrawCamera();
    SetupCameraParams(cam, cam);
    passConfig.viewport.x = 0;
    passConfig.viewport.y = 0;
    passConfig.viewport.width = Renderer::GetFramebufferWidth();
    passConfig.viewport.height = Renderer::GetFramebufferHeight();

    if (quadMaterial->PreBuildMaterial(passName)
        && quadHeatMaterial->PreBuildMaterial(passName)
        && BeginRenderPass())
    {
        ByndDynamicParams(cam);
        if (drawMode == eParticleDebugDrawMode::OVERDRAW)
            quadHeatMaterial->BindParams(quadPacket);
        else
            quadMaterial->BindParams(quadPacket);

        rhi::AddPacket(packetList, quadPacket);
        EndRenderPass();
    }
}
}
