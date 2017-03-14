#include "Particles/ParticleEffectDebug/ParticleDebugDrawQuadRenderPass.h"

#include "Render/RHI/rhi_Type.h"
#include "Render/RHI/rhi_Public.h"

namespace DAVA
{
const FastName ParticleDebugDrawQuadRenderPass::PASS_DEBUG_DRAW_QUAD("ForwardPass");

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
    if (quadBuffer.IsValid())
        rhi::DeleteVertexBuffer(quadBuffer);
}

void ParticleDebugDrawQuadRenderPass::Draw(DAVA::RenderSystem* renderSystem)
{
    Camera* cam = renderSystem->GetDrawCamera();
    SetupCameraParams(cam, cam);

    passConfig = renderSystem->GetMainPassConfig();
    passConfig.colorBuffer[0].loadAction = rhi::LOADACTION_LOAD;

    if (quadMaterial->PreBuildMaterial(passName)
        && quadHeatMaterial->PreBuildMaterial(passName)
        && BeginRenderPass())
    {
        if (drawMode == eParticleDebugDrawMode::OVERDRAW)
            quadHeatMaterial->BindParams(quadPacket);
        else
            quadMaterial->BindParams(quadPacket);

        rhi::AddPacket(packetList, quadPacket);
        EndRenderPass();
    }
}

void ParticleDebugDrawQuadRenderPass::PrepareRenderData()
{
    static const int vertsCount = 6;
    static const Array<ParticleDebugDrawQuadRenderPass::VertexPT, vertsCount> quad =
    { {
    { Vector3(-1.0f, -1.0f, 0.0f), Vector2(0.0f, 1.0f) },
    { Vector3(1.0f, -1.0f, 0.0f), Vector2(1.0f, 1.0f) },
    { Vector3(-1.0f, 1.0f, 0.0f), Vector2(0.0f, 0.0f) },
    { Vector3(-1.0f, 1.0f, 0.0f), Vector2(0.0f, 0.0f) },
    { Vector3(1.0f, -1.0f, 0.0f), Vector2(1.0f, 1.0f) },
    { Vector3(1.0f, 1.0f, 0.0f), Vector2(1.0f, 0.0f) }
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
}
