/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "Commands2/ParticleLayerMoveCommand.h"

ParticleLayerMoveCommand::ParticleLayerMoveCommand(DAVA::ParticleEmitterInstance* _oldEmitter, DAVA::ParticleLayer* _layer, DAVA::ParticleEmitterInstance* _newEmitter, DAVA::ParticleLayer* _newBefore /* = NULL */)
    : Command2(CMDID_PARTICLE_LAYER_MOVE, "Move particle layer")
    , layer(_layer)
    , oldEmitter(_oldEmitter)
    , newEmitter(_newEmitter)
    , newBefore(_newBefore)
{
    SafeRetain(layer);

    if ((layer != nullptr) && (oldEmitter != nullptr))
    {
        oldBefore = oldEmitter->GetEmitter()->GetNextLayer(layer);
    }
}

ParticleLayerMoveCommand::~ParticleLayerMoveCommand()
{
    SafeRelease(layer);
}

void ParticleLayerMoveCommand::Undo()
{
    if (layer == nullptr)
        return;

    if (nullptr != newEmitter)
    {
        newEmitter->GetEmitter()->RemoveLayer(layer);
    }

    if (nullptr != oldEmitter)
    {
        if (nullptr != oldBefore)
        {
            oldEmitter->GetEmitter()->InsertLayer(layer, oldBefore);
        }
        else
        {
            oldEmitter->GetEmitter()->AddLayer(layer);
        }
    }
}

void ParticleLayerMoveCommand::Redo()
{
    if ((layer == nullptr) || (newEmitter == nullptr))
        return;

    if (nullptr != oldEmitter)
    {
        oldEmitter->GetEmitter()->RemoveLayer(layer);
    }

    if (nullptr != newBefore)
    {
        newEmitter->GetEmitter()->InsertLayer(layer, newBefore);
    }
    else
    {
        newEmitter->GetEmitter()->AddLayer(layer);
    }
}
