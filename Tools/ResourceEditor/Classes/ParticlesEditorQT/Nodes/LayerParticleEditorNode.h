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



#ifndef __ResourceEditorQt__LayerParticleEditorNode__
#define __ResourceEditorQt__LayerParticleEditorNode__

#include "EmitterContainerNode.h"
#include "EmitterParticleEditorNode.h"
#include "Particles/ParticleEmitter.h"

namespace DAVA {
    
// Node which represents particle Layer node.
class LayerParticleEditorNode : public EmitterContainerNode
{
public:
    LayerParticleEditorNode(EmitterParticleEditorNode* emitterEditorNode,
                            ParticleLayer* layer);

    EmitterParticleEditorNode* GetEmitterEditorNode() const {return emitterEditorNode;};
    ParticleLayer* GetLayer() const {return layer;};

	virtual QString GetName() const;

    // Get the layer index in the Particle Emitter.
    int32 GetLayerIndex() const;

    // Get the count of forces added to the layer.
    int32 GetForcesCount() const;

	// Get the count of Inner Emitters added to the layer.
	int32 GetInnerEmittersCount();

    // Update the forces indices.
    void UpdateForcesIndices();
	
	// Update the Emitter Editor node.
	void UpdateEmitterEditorNode(EmitterParticleEditorNode* newNode)
	{
		this->emitterNode = newNode->GetEmitterNode();
		this->emitterEditorNode = newNode;
	};

protected:
    EmitterParticleEditorNode* emitterEditorNode;
    ParticleLayer* layer;
};

};

#endif /* defined(__ResourceEditorQt__LayerParticleEditorNode__) */
