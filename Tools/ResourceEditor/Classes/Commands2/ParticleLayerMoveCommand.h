#ifndef __PARTICLE_LAYER_MOVE_COMMAND_H__
#define __PARTICLE_LAYER_MOVE_COMMAND_H__

#include "QtTools/Commands/CommandWithoutExecute.h"
#include "Particles/ParticleLayer.h"
#include "Particles/ParticleEmitterInstance.h"

class ParticleLayerMoveCommand : public CommandWithoutExecute
{
public:
    ParticleLayerMoveCommand(DAVA::ParticleEmitterInstance* oldEmitter, DAVA::ParticleLayer* layer, DAVA::ParticleEmitterInstance* newEmitter, DAVA::ParticleLayer* newBefore = NULL);
    ~ParticleLayerMoveCommand();

    void Undo() override;
    void Redo() override;

private:
    DAVA::ParticleLayer* layer = nullptr;
    DAVA::ParticleEmitterInstance* oldEmitter = nullptr;
    DAVA::ParticleLayer* oldBefore = nullptr;
    DAVA::ParticleEmitterInstance* newEmitter = nullptr;
    DAVA::ParticleLayer* newBefore = nullptr;
};

#endif // __PARTICLE_LAYER_MOVE_COMMAND_H__
