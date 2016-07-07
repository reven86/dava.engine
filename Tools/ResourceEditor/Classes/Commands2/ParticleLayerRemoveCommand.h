#ifndef __PARTICLE_LAYER_REMOVE_COMMAND_H__
#define __PARTICLE_LAYER_REMOVE_COMMAND_H__

#include "Commands2/Base/RECommand.h"
#include "Particles/ParticleLayer.h"
#include "Particles/ParticleEmitter.h"

class ParticleLayerRemoveCommand : public RECommand
{
public:
    ParticleLayerRemoveCommand(DAVA::ParticleEmitter* emitter, DAVA::ParticleLayer* layer);
    ~ParticleLayerRemoveCommand();

    virtual void Undo();
    virtual void Redo();
    virtual DAVA::Entity* GetEntity() const
    {
        return NULL;
    }

    DAVA::ParticleLayer* layer;
    DAVA::ParticleLayer* before;
    DAVA::ParticleEmitter* emitter;
};

#endif // __PARTICLE_LAYER_REMOVE_COMMAND_H__
