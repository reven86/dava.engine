#include "Commands2/ParticleLayerRemoveCommand.h"
#include "Commands2/RECommandIDs.h"

ParticleLayerRemoveCommand::ParticleLayerRemoveCommand(DAVA::ParticleEmitter* _emitter, DAVA::ParticleLayer* _layer)
    : CommandWithoutExecute(CMDID_PARTICLE_LAYER_REMOVE, "Remove particle layer")
    , layer(_layer)
    , before(NULL)
    , emitter(_emitter)
{
    SafeRetain(layer);

    if ((NULL != layer) && (NULL != emitter))
    {
        before = emitter->GetNextLayer(layer);
    }
}

ParticleLayerRemoveCommand::~ParticleLayerRemoveCommand()
{
    SafeRelease(layer);
}

void ParticleLayerRemoveCommand::Undo()
{
    if (NULL != layer && NULL != emitter)
    {
        if (NULL != before)
        {
            emitter->InsertLayer(layer, before);
        }
        else
        {
            emitter->AddLayer(layer);
        }
    }
}

void ParticleLayerRemoveCommand::Redo()
{
    if (NULL != layer && NULL != emitter)
    {
        emitter->RemoveLayer(layer);
    }
}
