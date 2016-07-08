#ifndef __PARTICLE_FORCE_MOVE_COMMAND_H__
#define __PARTICLE_FORCE_MOVE_COMMAND_H__

#include "Commands2/Base/Command2.h"
#include "Particles/ParticleLayer.h"
#include "Particles/ParticleForce.h"

class ParticleForceMoveCommand : public Command2
{
public:
    ParticleForceMoveCommand(DAVA::ParticleForce* force, DAVA::ParticleLayer* oldLayer, DAVA::ParticleLayer* newLayer);
    ~ParticleForceMoveCommand();

    virtual void Undo();
    virtual void Redo();
    virtual DAVA::Entity* GetEntity() const
    {
        return NULL;
    }

    DAVA::ParticleForce* force;
    DAVA::ParticleLayer* oldLayer;
    DAVA::ParticleLayer* newLayer;
};

#endif // __PARTICLE_FORCE_MOVE_COMMAND_H__
