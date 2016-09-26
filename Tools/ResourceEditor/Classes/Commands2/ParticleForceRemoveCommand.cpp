#include "Commands2/ParticleForceRemoveCommand.h"
#include "Commands2/RECommandIDs.h"

ParticleForceRemoveCommand::ParticleForceRemoveCommand(DAVA::ParticleForce* _force, DAVA::ParticleLayer* _layer)
    : RECommand(CMDID_PARTICLE_LAYER_REMOVE, "Remove particle force")
    , force(_force)
    , layer(_layer)
{
    SafeRetain(force);
}

ParticleForceRemoveCommand::~ParticleForceRemoveCommand()
{
    SafeRelease(force);
}

void ParticleForceRemoveCommand::Undo()
{
    if (NULL != layer && NULL != force)
    {
        layer->AddForce(force);
    }
}

void ParticleForceRemoveCommand::Redo()
{
    if (NULL != layer && NULL != force)
    {
        layer->RemoveForce(force);
    }
}
