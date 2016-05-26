#include "Particles/ParticleEmitterInstance.h"

namespace DAVA
{
ParticleEmitterInstance::ParticleEmitterInstance(ParticleEffectComponent* owner_, bool isInner)
    : owner(owner_)
    , isInnerEmitter(isInner)
{
}

ParticleEmitterInstance::ParticleEmitterInstance(ParticleEffectComponent* owner_, ParticleEmitter* emitter_, bool isInner)
    : owner(owner_)
    , emitter(SafeRetain(emitter_))
    , isInnerEmitter(isInner)
{
}

ParticleEmitterInstance* ParticleEmitterInstance::Clone() const
{
    ScopedPtr<ParticleEmitter> clonedEmitter(emitter->Clone());
    ParticleEmitterInstance* result = new ParticleEmitterInstance(owner, clonedEmitter.get());
    result->SetFilePath(GetFilePath());
    result->SetSpawnPosition(GetSpawnPosition());
    result->isInnerEmitter = isInnerEmitter;
    return result;
}
}
