#include "Commands2/ParticleEmitterMoveCommands.h"

ParticleEmitterMoveCommand::ParticleEmitterMoveCommand(DAVA::ParticleEffectComponent* oldEffect_, DAVA::ParticleEmitterInstance* emitter_,
                                                       DAVA::ParticleEffectComponent* newEffect_, int newIndex_)
    : Command2(CMDID_PARTICLE_EMITTER_MOVE, "Move particle emitter")
    , oldEffect(oldEffect_)
    , newEffect(newEffect_)
    , oldIndex(-1)
    , newIndex(newIndex_)
{
    if (nullptr != emitter_ && nullptr != oldEffect)
    {
        oldIndex = oldEffect->GetEmitterInstanceIndex(emitter_);
        instance = oldEffect->GetEmitterInstance(oldIndex);
        DVASSERT(instance->GetEmitter() == emitter_->GetEmitter());
    }
}

void ParticleEmitterMoveCommand::Undo()
{
    if ((instance == nullptr) || (instance->GetEmitter() == nullptr))
        return;

    if (nullptr != newEffect)
    {
        newEffect->RemoveEmitterInstance(instance);
    }

    if (nullptr != oldEffect)
    {
        if (-1 != oldIndex)
        {
            oldEffect->InsertEmitterInstanceAt(instance, oldIndex);
        }
        else
        {
            oldEffect->AddEmitterInstance(instance);
        }
    }
}

void ParticleEmitterMoveCommand::Redo()
{
    if ((instance == nullptr) || (instance->GetEmitter() == nullptr) || (newEffect == nullptr))
        return;

    if (nullptr != oldEffect)
    {
        oldEffect->RemoveEmitterInstance(instance);
    }

    if (-1 != newIndex)
    {
        newEffect->InsertEmitterInstanceAt(instance, newIndex);
    }
    else
    {
        newEffect->AddEmitterInstance(instance);
    }
}
