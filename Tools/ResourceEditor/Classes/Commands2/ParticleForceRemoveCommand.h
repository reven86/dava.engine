#ifndef __PARTICLE_FORCE_REMOVE_COMMAND_H__
#define __PARTICLE_FORCE_REMOVE_COMMAND_H__

#include "QtTools/Commands/CommandWithoutExecute.h"
#include "Particles/ParticleLayer.h"
#include "Particles/ParticleForce.h"

class ParticleForceRemoveCommand : public CommandWithoutExecute
{
public:
    ParticleForceRemoveCommand(DAVA::ParticleForce* force, DAVA::ParticleLayer* layer);
    ~ParticleForceRemoveCommand();

    void Undo() override;
    void Redo() override;

    DAVA::ParticleForce* force;
    DAVA::ParticleLayer* layer;
};

#endif // __PARTICLE_FORCE_REMOVE_COMMAND_H__
