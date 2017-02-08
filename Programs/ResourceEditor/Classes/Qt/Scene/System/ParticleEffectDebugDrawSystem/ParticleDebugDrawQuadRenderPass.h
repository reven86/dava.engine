#pragma once

#include "Render/Highlevel/RenderPass.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Base/FastName.h"

enum eParticleDebugDrawMode;

class ParticleDebugDrawQuadRenderPass : public DAVA::RenderPass
{
public:
    struct ParticleDebugQuadRenderPassConfig
    {
        const DAVA::FastName& name;
        RenderSystem* renderSystem;
        DAVA::NMaterial* quadMaterial;
        DAVA::NMaterial* quadHeatMaterial;
        const eParticleDebugDrawMode& drawMode;
    };
    ParticleDebugDrawQuadRenderPass(ParticleDebugQuadRenderPassConfig config);
    ~ParticleDebugDrawQuadRenderPass();
    void Draw(DAVA::RenderSystem* renderSystem) override;
    static const DAVA::FastName PASS_DEBUG_DRAW_QUAD;

private:    
    struct VertexPT
    {
        Vector3 position;
        Vector2 uv;
    };
    void PrepareRenderData();

    DAVA::NMaterial* quadMaterial;
    DAVA::NMaterial* quadHeatMaterial;

    const eParticleDebugDrawMode& drawMode;
    
    rhi::HVertexBuffer quadBuffer;

    rhi::Packet quadPacket;

    void ByndDynamicParams(Camera* cam);
};
