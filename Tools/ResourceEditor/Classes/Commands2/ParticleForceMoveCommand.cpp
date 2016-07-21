#include "Commands2/ParticleForceMoveCommand.h"
#include "Commands2/RECommandIDs.h"

ParticleForceMoveCommand::ParticleForceMoveCommand(DAVA::ParticleForce* _force, DAVA::ParticleLayer* _oldLayer, DAVA::ParticleLayer* _newLayer)
    : RECommand(CMDID_PARTICLE_FORCE_MOVE, "Move particle force")
    , force(_force)
    , oldLayer(_oldLayer)
    , newLayer(_newLayer)
{
    SafeRetain(force);
}

ParticleForceMoveCommand::~ParticleForceMoveCommand()
{
    SafeRelease(force);
}

void ParticleForceMoveCommand::Undo()
{
    if (NULL != force)
    {
        if (NULL != newLayer)
        {
            newLayer->RemoveForce(force);
        }

        if (NULL != oldLayer)
        {
            oldLayer->AddForce(force);
        }
    }
}

void ParticleForceMoveCommand::Redo()
{
    if (NULL != force)
    {
        if (NULL != oldLayer)
        {
            oldLayer->RemoveForce(force);
        }

        if (NULL != newLayer)
        {
            newLayer->AddForce(force);
        }
    }
}
