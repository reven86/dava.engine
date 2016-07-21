#ifndef __PARTICLE_FORCE_MOVE_COMMAND_H__
#define __PARTICLE_FORCE_MOVE_COMMAND_H__

#include "Commands2/Base/RECommand.h"
#include "Particles/ParticleLayer.h"
#include "Particles/ParticleForce.h"

class ParticleForceMoveCommand : public RECommand
{
public:
    ParticleForceMoveCommand(DAVA::ParticleForce* force, DAVA::ParticleLayer* oldLayer, DAVA::ParticleLayer* newLayer);
    ~ParticleForceMoveCommand();

    void Undo() override;
    void Redo() override;

    DAVA::ParticleForce* force;
    DAVA::ParticleLayer* oldLayer;
    DAVA::ParticleLayer* newLayer;
};

#endif // __PARTICLE_FORCE_MOVE_COMMAND_H__
