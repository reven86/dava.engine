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



#include "Particles/ParticleEmitter.h"
#include "Particles/ParticleLayer.h"

#include "LayerParticleEditorNode.h"
#include "ForceParticleEditorNode.h"
#include "InnerEmitterParticleEditorNode.h"

using namespace DAVA;

LayerParticleEditorNode::LayerParticleEditorNode(EmitterParticleEditorNode* emitterEditorNode,
                                                 ParticleLayer* layer) :
    EmitterContainerNode(emitterEditorNode->GetRootNode(), emitterEditorNode->GetEmitterNode(),
                         "Emitter Layer")
{
    this->emitterEditorNode = emitterEditorNode;
    this->layer = layer;
}

QString LayerParticleEditorNode::GetName() const
{
	if (this->layer)
	{
		return QString::fromStdString(this->layer->layerName);
	}
	
	return QString();
}

int32 LayerParticleEditorNode::GetLayerIndex() const
{
    ParticleEmitter* emitter = GetParticleEmitter();
    if (!emitter)
    {
        return -1;
    }
    
    const Vector<ParticleLayer*>& layers = emitter->GetLayers();
    int32 layersCount = layers.size();
    
    for (int32 i = 0; i < layersCount; i ++)
    {
        if (layers[i] == this->layer)
        {
            return i;
        }
    }
    
    return -1;
}

int32 LayerParticleEditorNode::GetForcesCount() const
{
    int32 forcesCount = 0;
    for (List<BaseParticleEditorNode*>::const_iterator iter = GetChildren().begin(); iter != GetChildren().end();
         iter ++)
    {
        ForceParticleEditorNode* childNode = dynamic_cast<ForceParticleEditorNode*>(*iter);
        if (childNode)
        {
            forcesCount ++;
        }
    }
    
    return forcesCount;
}

void LayerParticleEditorNode::UpdateForcesIndices()
{
    // Re-synchronize new Forces List with the appropriate Tree Nodes.
    ParticleLayer* curLayer = this->GetLayer();
    
    int32 processedChildNodes = 0;
    for (List<BaseParticleEditorNode*>::const_iterator iter = GetChildren().begin(); iter != GetChildren().end();
         iter ++)
    {
        ForceParticleEditorNode* childNode = dynamic_cast<ForceParticleEditorNode*>(*iter);
        if (!childNode)
        {
            continue;
        }
        
        childNode->UpdateForceIndex(processedChildNodes);
        childNode->SetNodeName(QString("Force %1").arg(processedChildNodes + 1));
        processedChildNodes ++;
    }

    // We have to update exactly the same child nodes as the forces count we have.
    int32 forcesCount = curLayer->forces.size();
    DVASSERT(forcesCount == processedChildNodes);
}

int32 LayerParticleEditorNode::GetInnerEmittersCount()
{
	int32 innerEmittersCount = 0;
    for (List<BaseParticleEditorNode*>::const_iterator iter = GetChildren().begin(); iter != GetChildren().end();
         iter ++)
    {
        InnerEmitterParticleEditorNode* childNode = dynamic_cast<InnerEmitterParticleEditorNode*>(*iter);
        if (childNode)
        {
            innerEmittersCount ++;
        }
    }
    
    return innerEmittersCount;
}