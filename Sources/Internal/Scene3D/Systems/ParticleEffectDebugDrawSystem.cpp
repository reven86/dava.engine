#include "Scene3D/Systems/ParticleEffectDebugDrawSystem.h"

#include "Particles/ParticleEffectDebug/ParticleDebugRenderPass.h"
#include "Particles/ParticleEffectDebug/ParticleDebugDrawQuadRenderPass.h"

#include "Render/Highlevel/RenderSystem.h"

#include "Scene3D/Scene.h"

namespace DAVA
{
ParticleEffectDebugDrawSystem::ParticleEffectDebugDrawSystem(Scene* scene)
    : SceneSystem(scene)
{
    if (scene != nullptr)
    {
        GenerateDebugMaterials();

        renderSystem = scene->GetRenderSystem();

        ParticleDebugRenderPass::ParticleDebugRenderPassConfig config =
        { ParticleDebugRenderPass::PASS_DEBUG_DRAW_PARTICLES, renderSystem, wireframeMaterial, overdrawMaterial,
          showAlphaMaterial, drawMode, isDrawOnlySelected, &selectedParticles };
        renderPass = new ParticleDebugRenderPass(config);

        heatTexture = GenerateHeatTexture();
        GenerateQuadMaterials();

        ParticleDebugDrawQuadRenderPass::ParticleDebugQuadRenderPassConfig quadPassConfig =
        { ParticleDebugDrawQuadRenderPass::PASS_DEBUG_DRAW_QUAD, renderSystem, quadMaterial, quadHeatMaterial, drawMode };
        drawQuadPass = new ParticleDebugDrawQuadRenderPass(quadPassConfig);
    }
    materials.push_back(wireframeMaterial);
    materials.push_back(overdrawMaterial);
    materials.push_back(showAlphaMaterial);
    materials.push_back(quadMaterial);
    materials.push_back(quadHeatMaterial);
}

ParticleEffectDebugDrawSystem::~ParticleEffectDebugDrawSystem()
{
    SafeDelete(renderPass);
    SafeDelete(drawQuadPass);

    SafeRelease(wireframeMaterial);
    SafeRelease(overdrawMaterial);
    SafeRelease(showAlphaMaterial);

    SafeRelease(quadMaterial);
    SafeRelease(quadHeatMaterial);

    SafeRelease(heatTexture);
    materials.clear();
}

void ParticleEffectDebugDrawSystem::Draw()
{
    renderPass->Draw(renderSystem);
    drawQuadPass->Draw(renderSystem);
}

void ParticleEffectDebugDrawSystem::GenerateDebugMaterials()
{
    if (wireframeMaterial == nullptr)
    {
        wireframeMaterial = new NMaterial();
        wireframeMaterial->SetFXName(NMaterialName::DEBUG_DRAW_WIREFRAME);

        Color wireframeColor(0.0f, 0.0f, 0.0f, 1.0f);
        wireframeMaterial->AddProperty(FastName("color"), wireframeColor.color, rhi::ShaderProp::TYPE_FLOAT4);
        wireframeMaterial->AddFlag(NMaterialFlagName::FLAG_BLENDING, eBlending::BLENDING_ALPHABLEND);
    }
    if (overdrawMaterial == nullptr)
    {
        overdrawMaterial = new NMaterial();
        overdrawMaterial->SetFXName(NMaterialName::PARTICLES);
        overdrawMaterial->AddFlag(FastName("PARTICLE_DEBUG_SHOW_OVERDRAW"), true);
        overdrawMaterial->AddFlag(NMaterialFlagName::FLAG_BLENDING, eBlending::BLENDING_ADDITIVE);
    }
    if (showAlphaMaterial == nullptr)
    {
        float alphaThreshold = 0.05f;

        showAlphaMaterial = new NMaterial();
        showAlphaMaterial->SetFXName(NMaterialName::PARTICLES);
        showAlphaMaterial->AddFlag(FastName("PARTICLE_DEBUG_SHOW_ALPHA"), true);
        showAlphaMaterial->AddFlag(NMaterialFlagName::FLAG_BLENDING, eBlending::BLENDING_ALPHABLEND);
        showAlphaMaterial->AddProperty(FastName("particleAlphaThreshold"), &alphaThreshold, rhi::ShaderProp::TYPE_FLOAT1);
    }
}

void ParticleEffectDebugDrawSystem::GenerateQuadMaterials()
{
    if (quadMaterial == nullptr)
    {
        quadMaterial = new NMaterial();
        quadMaterial->SetFXName(NMaterialName::DEBUG_DRAW_PARTICLES);
        quadMaterial->AddTexture(NMaterialTextureName::TEXTURE_PARTICLES_RT, renderPass->GetTexture());
        quadMaterial->AddFlag(NMaterialFlagName::FLAG_BLENDING, eBlending::BLENDING_ALPHABLEND);
    }

    if (quadHeatMaterial == nullptr)
    {
        quadHeatMaterial = new NMaterial();
        quadHeatMaterial->SetFXName(NMaterialName::DEBUG_DRAW_PARTICLES);
        quadHeatMaterial->AddTexture(NMaterialTextureName::TEXTURE_PARTICLES_HEATMAP, heatTexture);
        quadHeatMaterial->AddTexture(NMaterialTextureName::TEXTURE_PARTICLES_RT, renderPass->GetTexture());
        quadHeatMaterial->AddFlag(NMaterialFlagName::FLAG_PARTICLES_DEBUG_SHOW_OVERDRAW, 1);
        quadHeatMaterial->AddFlag(NMaterialFlagName::FLAG_BLENDING, eBlending::BLENDING_ALPHABLEND);
    }
}

DAVA::Texture* ParticleEffectDebugDrawSystem::GenerateHeatTexture()
{
    static const int32 width = 32;
    static const int32 height = 1;
    static const size_t dataSize = width * height * 4;

    unsigned char* data = new unsigned char[dataSize];
    for (size_t i = 0; i < dataSize; i += 4)
    {
        Vector4 color = LerpColors(static_cast<float32>(i) / (4 * width));
        data[i] = static_cast<uint8>(color.x);
        data[i + 1] = static_cast<uint8>(color.y);
        data[i + 2] = static_cast<uint8>(color.z);
        data[i + 3] = static_cast<uint8>(color.w);
    }
    Texture* texture = Texture::CreateFromData(FORMAT_RGBA8888, data, width, height, false);

    delete[] data;

    return texture;
}

DAVA::Vector4 ParticleEffectDebugDrawSystem::LerpColors(float normalizedWidth)
{
    static const Vector<TextureKey> keys =
    {
      TextureKey(Vector4(0.0f, 0.0f, 0.0f, 0.0f), 0.f),
      TextureKey(Vector4(0.0f, 255.0f, 0.0f, 128.0f), 0.001f),
      TextureKey(Vector4(0.0f, 255.0f, 0.0f, 255.0f), 0.06f),
      TextureKey(Vector4(100.0f, 100.0f, 0.0f, 255.0f), 0.2f),
      TextureKey(Vector4(255.0f, 80.0f, 0.0f, 255.0f), 0.4f),
      TextureKey(Vector4(255.0f, 64.0f, 0.0f, 255.0f), 0.5f),
      TextureKey(Vector4(255.0f, 0.0f, 0.0f, 255.0f), 1.0f)
    };
    const TextureKey* current = nullptr;
    const TextureKey* next = nullptr;
    for (size_t i = 0; i < keys.size() - 1; i++)
        if (keys[i].time <= normalizedWidth && keys[i + 1].time >= normalizedWidth)
        {
            current = &keys[i];
            next = &keys[i + 1];
        }
    if (current == nullptr)
        return Vector4(0, 0, 0, 0);

    float t = (normalizedWidth - current->time) / (next->time - current->time);
    return Lerp(current->color, next->color, t);
}

void ParticleEffectDebugDrawSystem::SetSelectedParticles(DAVA::UnorderedSet<RenderObject*> selectedParticles_)
{
    selectedParticles = selectedParticles_;
}

void ParticleEffectDebugDrawSystem::SetAlphaThreshold(float32 threshold)
{
    threshold = Clamp(threshold, 0.0f, 1.0f);
    if (showAlphaMaterial != nullptr)
        showAlphaMaterial->SetPropertyValue(FastName("particleAlphaThreshold"), &threshold);
}
}