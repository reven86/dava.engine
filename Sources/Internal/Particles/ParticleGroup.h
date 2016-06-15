#ifndef __DAVAENGINE_PARTICLE_GROUP_H_
#define __DAVAENGINE_PARTICLE_GROUP_H_

#include "ParticleEmitter.h"
#include "ParticleLayer.h"
#include "Particle.h"
#include "Render/Material/NMaterial.h"

namespace DAVA
{
struct ParticleGroup
{
    ParticleEmitter* emitter = nullptr;
    ParticleLayer* layer = nullptr;
    NMaterial* material = nullptr;
    Particle* head = nullptr;

    Vector3 spawnPosition;

    int32 positionSource = 0;
    int32 activeParticleCount = 0;

    float32 time = 0.0f;
    float32 loopStartTime = 0.0f;
    float32 loopLyaerStartTime = 0.0f;
    float32 loopDuration = 0.0f;
    float32 particlesToGenerate = 0.0f;

    bool finishingGroup = false;
    bool visibleLod = true;
};

struct ParentInfo
{
    Vector3 position;
    Vector2 size;
};

struct ParticleEffectData
{
    Vector<ParentInfo> infoSources;
    List<ParticleGroup> groups;
};
}

#endif
