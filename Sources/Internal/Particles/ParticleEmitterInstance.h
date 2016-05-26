#ifndef __DAVAENGINE_PARTICLE_EMITTER_INSTANCE_H__
#define __DAVAENGINE_PARTICLE_EMITTER_INSTANCE_H__

#include "Particles/ParticleEmitter.h"

namespace DAVA
{
class ParticleEffectComponent;
class ParticleEmitterInstance : public BaseObject
{
public:
    ParticleEmitterInstance(ParticleEffectComponent* owner, bool isInner = false);
    ParticleEmitterInstance(ParticleEffectComponent* owner, ParticleEmitter* emitter, bool isInner = false);

    ParticleEmitter* GetEmitter() const;
    const FilePath& GetFilePath() const;
    const Vector3& GetSpawnPosition() const;

    void SetEmitter(ParticleEmitter* emitter);
    void SetFilePath(const FilePath& filePath);
    void SetSpawnPosition(const Vector3& position);
    void SetOwner(ParticleEffectComponent* owner);

    ParticleEmitterInstance* Clone() const;
    ParticleEffectComponent* GetOwner() const;

    bool IsInnerEmitter() const;

private:
    ParticleEffectComponent* owner = nullptr;
    RefPtr<ParticleEmitter> emitter;
    FilePath filePath;
    Vector3 spawnPosition;
    bool isInnerEmitter = false;

public:
    INTROSPECTION_EXTEND(ParticleEmitterInstance, BaseObject, nullptr)
};

inline ParticleEmitter* ParticleEmitterInstance::GetEmitter() const
{
    return emitter.Get();
}

inline const FilePath& ParticleEmitterInstance::GetFilePath() const
{
    return filePath;
}

inline const Vector3& ParticleEmitterInstance::GetSpawnPosition() const
{
    return spawnPosition;
}

inline ParticleEffectComponent* ParticleEmitterInstance::GetOwner() const
{
    return owner;
}

inline bool ParticleEmitterInstance::IsInnerEmitter() const
{
    return isInnerEmitter;
}

inline void ParticleEmitterInstance::SetEmitter(ParticleEmitter* _emitter)
{
    emitter.Set(_emitter);
}

inline void ParticleEmitterInstance::SetFilePath(const FilePath& _filePath)
{
    filePath = _filePath;
}

inline void ParticleEmitterInstance::SetSpawnPosition(const Vector3& _position)
{
    spawnPosition = _position;
}

inline void ParticleEmitterInstance::SetOwner(ParticleEffectComponent* owner_)
{
    owner = owner_;
}
}
#endif // __DAVAENGINE_PARTICLE_EMITTER_INSTANCE_H__
