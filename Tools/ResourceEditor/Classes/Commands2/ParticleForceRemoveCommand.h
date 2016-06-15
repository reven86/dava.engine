#ifndef __PARTICLE_FORCE_REMOVE_COMMAND_H__
#define __PARTICLE_FORCE_REMOVE_COMMAND_H__

#include "Commands2/Base/Command2.h"
#include "Particles/ParticleLayer.h"
#include "Particles/ParticleForce.h"

class ParticleForceRemoveCommand : public Command2
{
public:
    ParticleForceRemoveCommand(DAVA::ParticleForce* force, DAVA::ParticleLayer* layer);
    ~ParticleForceRemoveCommand();

    virtual void Undo();
    virtual void Redo();
    virtual DAVA::Entity* GetEntity() const
    {
        return NULL;
    }

    DAVA::ParticleForce* force;
    DAVA::ParticleLayer* layer;
};

#endif // __PARTICLE_FORCE_REMOVE_COMMAND_H__
