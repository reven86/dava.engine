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



#ifndef __ResourceEditorQt__ParticlesEditorController__
#define __ResourceEditorQt__ParticlesEditorController__

#include "DAVAEngine.h"
#include "Base/Singleton.h"

#include "DockSceneGraph/SceneGraphItem.h"
#include "Scene3D/ParticleEffectNode.h"

#include "ParticlesEditorQT/Nodes/BaseParticleEditorNode.h"
#include "ParticlesEditorQT/Nodes/EffectParticleEditorNode.h"
#include "ParticlesEditorQT/Nodes/EmitterParticleEditorNode.h"
#include "ParticlesEditorQT/Nodes/LayerParticleEditorNode.h"
#include "ParticlesEditorQT/Nodes/ForceParticleEditorNode.h"

#include <QObject>

namespace DAVA {
    
// Particles Editor Controller.
class ParticlesEditorController : public QObject, public Singleton<ParticlesEditorController>
{
    Q_OBJECT
public:
    explicit ParticlesEditorController(QObject* parent = 0);
    virtual ~ParticlesEditorController();
    
    // Register/remove the Patricles Effect node.
    EffectParticleEditorNode* RegisterParticleEffectNode(Entity* effectNode, bool autoStart = true);
    void UnregiserParticleEffectNode(Entity* effectNode);

    // Whether the node belongs to Particle Editor?
    bool IsBelongToParticlesEditor(SceneGraphItem* sceneGraphItem);
    
    // For some inner nodes (like Particle Emitter one) we have to display
    // its properties in the Scene Editor. This method decides for which exactly.
    bool ShouldDisplayPropertiesInSceneEditor(SceneGraphItem *sceneGraphItem);

    // Work with selected node.
    void SetSelectedNode(SceneGraphItem* selectedItem, bool isEmitEvent);
    BaseParticleEditorNode* GetSelectedNode() const {return selectedNode;};
    void CleanupSelectedNode();

    // Get the Root node for the Particle Effect registered.
    EffectParticleEditorNode* GetRootForParticleEffectNode(Entity* effectNode);
    
    // Add/remove different types of Particles Editor nodes to the scene.
    void AddParticleEmitterNodeToScene(Entity* emitterSceneNode);
    void RemoveParticleEmitterNode(Entity* emitterSceneNode);
    
    ForceParticleEditorNode* AddParticleForceToNode(LayerParticleEditorNode* layerNode);
    void RemoveParticleForceNode(ForceParticleEditorNode* forceNode);

	// Move different nodes logic.
	// Move the emitters inside the same layer.
	bool MoveEmitter(EmitterParticleEditorNode* movedItemEmitterNode,
					 EmitterParticleEditorNode* newItemEmitterNode);

	// Move the emitter from one effect to another one.
	bool MoveEmitter(EmitterParticleEditorNode* movedItemEmitterNode,
					 EffectParticleEditorNode* newEffectParentNode,
					 EmitterParticleEditorNode* newEffectReferenceEmitterNode);
	
	// Move the Layer between Emitters or inside the same Emitter.
	bool MoveLayer(LayerParticleEditorNode* movedItemNode, LayerParticleEditorNode* beforeNode);

	// Move the Layer to the end of another Emitter.
	bool MoveLayer(LayerParticleEditorNode* moveItemNode, EmitterParticleEditorNode* newEmitterNode);

	// Sprites packer entry point.
	void PackSprites();

	// Refresh the selected node (called if something is changed outside).
	void RefreshSelectedNode(bool forceRefresh = false);

signals:
	void EffectSelected(Entity* effectNode);
    void EmitterSelected(Entity* emitterNode, BaseParticleEditorNode* editorNode);
	void LayerSelected(Entity* emitterNode, ParticleLayer* layer, BaseParticleEditorNode* editorNode, bool forceRefresh);
    void ForceSelected(Entity* emitterNode, ParticleLayer* layer, int32 forceIndex, BaseParticleEditorNode* editorNode);
	void NodeDeselected(BaseParticleEditorNode* editorNode);

protected:
    // Emit the selected node changed.
    void EmitSelectedNodeChanged(bool forceRefresh = true);
	void EmitNodeWillBeDeselected();

	// Find the Emitter Editor node by the appropriate Scene Node.
    void FindEmitterEditorNode(Entity* emitterSceneNode,
                               EffectParticleEditorNode** rootEditorNode,
                               EmitterParticleEditorNode** emitterEditorNode);

    // Cleanup the memory used.
    void Cleanup();

	// Change the layers order for the same Particle Emitter node.
	// Move the layer in same or different Emitter Nodes.
	bool ChangeLayersOrderInSameEmitter(LayerParticleEditorNode* movedItemNode, LayerParticleEditorNode* beforeNode);
	
	bool ChangeLayersOrderInDifferentEmitters(LayerParticleEditorNode* moveItemNode, LayerParticleEditorNode* beforeNode);

	// Move the layer to the end of the new emitter node.
	bool MoveLayerToEmitter(LayerParticleEditorNode* moveItemNode, EmitterParticleEditorNode* newEmitterNode);

	// Common function to move layer between emitters.
	bool PerformMoveBetweenEmitters(EmitterParticleEditorNode* oldEmitterNode, EmitterParticleEditorNode* newEmitterNode,
									LayerParticleEditorNode* layerToMove,LayerParticleEditorNode* layerToInsertBefore);

	// Cleanup the selected node in case it is one to be deleted.
	void CleanupSelectedNodeIfDeleting(BaseParticleEditorNode* nodeToBeDeleted);

    // Particle Effects registered in the system.
    typedef Map<Entity*, EffectParticleEditorNode*> PARTICLESEFFECTMAP;
    typedef PARTICLESEFFECTMAP::iterator PARTICLESEFFECTITER;
    
    PARTICLESEFFECTMAP particleEffectNodes;

    // Selected node.
    BaseParticleEditorNode* selectedNode;
};
    
}

#endif /* defined(__ResourceEditorQt__ParticlesEditorController__) */
