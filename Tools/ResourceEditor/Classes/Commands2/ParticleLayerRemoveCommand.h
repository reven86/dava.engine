#ifndef __PARTICLE_LAYER_REMOVE_COMMAND_H__
#define __PARTICLE_LAYER_REMOVE_COMMAND_H__

#include "QtTools/Commands/CommandWithoutExecute.h"
#include "Particles/ParticleLayer.h"
#include "Particles/ParticleEmitter.h"

class ParticleLayerRemoveCommand : public CommandWithoutExecute
{
public:
    ParticleLayerRemoveCommand(DAVA::ParticleEmitter* emitter, DAVA::ParticleLayer* layer);
    ~ParticleLayerRemoveCommand();

    virtual void Undo() override;
    virtual void Redo() override;

    DAVA::ParticleLayer* layer;
    DAVA::ParticleLayer* before;
    DAVA::ParticleEmitter* emitter;
};

#endif // __PARTICLE_LAYER_REMOVE_COMMAND_H__
