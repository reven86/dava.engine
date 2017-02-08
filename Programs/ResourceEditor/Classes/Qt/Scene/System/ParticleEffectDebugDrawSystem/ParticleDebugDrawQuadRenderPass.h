#pragma once

#include "Render/Highlevel/RenderPass.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Base/FastName.h"

class ParticleDebugDrawQuadRenderPass : public DAVA::RenderPass
{
public:
    ParticleDebugDrawQuadRenderPass(const DAVA::FastName& name, DAVA::RenderSystem* renderSystem, DAVA::Texture* texHandle);
    ~ParticleDebugDrawQuadRenderPass();
    void Draw(DAVA::RenderSystem* renderSystem) override;
    static const DAVA::FastName PASS_DEBUG_DRAW_QUAD;

private:    
    struct VertexPT
    {
        Vector3 position;
        Vector2 uv;
    };

    DAVA::Texture* debugTexture;
    //static constexpr int VERTEX_COUNT = 6;
    static const DAVA::Array<VertexPT, 6> quad;
    DAVA::NMaterial* quadMaterial;
    void PrepareRenderData();
    
    rhi::HVertexBuffer quadBuffer;

    rhi::Packet quadPacket;

    void ByndDynamicParams(Camera* cam);

};
