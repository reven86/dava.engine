#ifndef __PARTICLE_EMITTER_MOVE_COMMANDS_H__
#define __PARTICLE_EMITTER_MOVE_COMMANDS_H__

#include "Commands2/Base/Command2.h"
#include "Scene3D/Components/ParticleEffectComponent.h"
#include "Particles/ParticleEmitter.h"

class ParticleEmitterMoveCommand : public Command2
{
public:
    ParticleEmitterMoveCommand(DAVA::ParticleEffectComponent* oldEffect, DAVA::ParticleEmitterInstance* emitter, DAVA::ParticleEffectComponent* newEffect, int newIndex);

    void Undo() override;
    void Redo() override;
    DAVA::Entity* GetEntity() const override;

    DAVA::ParticleEmitterInstance* instance = nullptr;
    DAVA::ParticleEffectComponent* oldEffect = nullptr;
    DAVA::ParticleEffectComponent* newEffect = nullptr;
    DAVA::int32 oldIndex = -1;
    DAVA::int32 newIndex;
};

inline DAVA::Entity* ParticleEmitterMoveCommand::GetEntity() const
{
    return nullptr;
}

#endif