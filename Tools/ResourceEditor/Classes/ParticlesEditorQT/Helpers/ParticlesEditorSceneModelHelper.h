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



#ifndef __ResourceEditorQt__ParticlesEditorSceneModelHelper__
#define __ResourceEditorQt__ParticlesEditorSceneModelHelper__

#include "DAVAEngine.h"
#include <QItemSelection>
#include <QModelIndex>
#include <QMenu>

#include "ParticlesEditorQT/Nodes/BaseParticleEditorNode.h"
#include "ParticlesEditorQT/Nodes/EffectParticleEditorNode.h"
#include "ParticlesEditorQT/Nodes/EmitterParticleEditorNode.h"
#include "ParticlesEditorQT/Nodes/LayerParticleEditorNode.h"
#include "ParticlesEditorQT/Nodes/ForceParticleEditorNode.h"
#include "ParticlesEditorQT/Nodes/InnerEmitterParticleEditorNode.h"

#include "Scene/SceneData.h"
#include "DockSceneGraph/SceneGraphItem.h"

#include "Commands2/Command2.h"

class SceneGraphModel;

namespace DAVA {

// Scene Model Helper for Particles Editor.
class ParticlesEditorSceneModelHelper
{
public:
    // Custom processing the Selection Changed in the Scene Graph model. Returns
    // TRUE if no further processing needed.
    bool ProcessSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    // Custom node selection.
    SceneGraphItem* GetGraphItemToBeSelected(GraphItem* rootItem, Entity* node);
	
	// Reset the selected item.
	void ResetSelection();

    // Preprocess the Scene Node during adding, change its type if needed.
    Entity* PreprocessSceneNode(Entity* rawNode);

    // Add the node and all its children to the Scene Graph.
    bool AddNodeToSceneGraph(SceneGraphItem *graphItem, Entity *node);

    // Whether we need to display "standard" Scene Editor Popup Menu for this node?
    bool NeedDisplaySceneEditorPopupMenuItems(const QModelIndex &index) const;

    // Add Popup Menu items depending on the tree node selected.
    void AddPopupMenuItems(QMenu &menu, const QModelIndex &index) const;
    
	// Move the item to parent functionality.
	// Whether the move should be handled by the Particle Editor?
	bool NeedMoveItemToParent(GraphItem* movedItem, GraphItem* newParentItem);

	// Perform the move itself.
	bool MoveItemToParent(GraphItem* movedItem, GraphItem* newParentItem);

	// Get the Scene Graph item which contains the appropriate Particles Editor objects.
	SceneGraphItem* GetGraphItemForParticlesLayer(GraphItem* rootItem, DAVA::ParticleLayer* layer);
	
	// Get the persistent pointer to the Particle Editor element.
	void* GetPersistentDataForModelIndex(const QModelIndex& modelIndex) const;

	// Some Scene Graph tree nodes might be checkable - check logic.
	// Is the graph item passed checkable?
	bool IsGraphItemCheckable(GraphItem* graphItem) const;
	
	// Get/set the checked state for the Graph Item, if it is checkable.
	bool GetCheckableStateForGraphItem(GraphItem* graphItem) const;
	void SetCheckableStateForGraphItem(GraphItem* graphItem, bool value);
	
	// Update the representation of the layer when it is refreshed.
	void UpdateLayerRepresentation(GraphItem* rootItem,
								   DAVA::ParticleLayer* layer,
								   SceneGraphModel* sceneGraphModel);

protected:
	// Add the action to QT menu.
	void AddActionToMenu(QMenu *menu, const QString &actionTitle, Command2 *command) const;
	
    // Build the Scene Graph in a recursive way.
    void BuildSceneGraphRecursive(BaseParticleEditorNode* rootNode, SceneGraphItem* rootItem);

    // Determine whether we need to select Editor Tree Node in a recursive way.
    SceneGraphItem* GetGraphItemToBeSelectedRecursive(GraphItem* rootItem, Entity* node);

    // Do the checks needed and return ExtraUserData from model index. or from the item.
    ExtraUserData* GetExtraUserData(const QModelIndex& modelIndex) const;
	ExtraUserData* GetExtraUserData(GraphItem* item) const;
	ExtraUserData* GetExtraUserDataByModelIndex(const QModelIndex& modelIndex) const;
    
    // Synchronization of the whole Particle Editor Tree and different types of Nodes.
    void SynchronizeParticleEditorTree(BaseParticleEditorNode* node);

    void SynchronizeEffectParticleEditorNode(EffectParticleEditorNode* node, Entity* effectRootNode);
    void SynchronizeEmitterParticleEditorNode(EmitterParticleEditorNode* node);
    void SynchronizeLayerParticleEditorNode(LayerParticleEditorNode* node);
	void SynchronizeInnerEmitterNode(LayerParticleEditorNode* node, SceneGraphItem* layerNodeItem = NULL, SceneGraphModel* sceneGraphModel = NULL);

	void BuildEntitiesSets(EffectParticleEditorNode* node, Entity* effectRootNode,
						   Set<Entity*>& entitiesInParticleEditor,
						   Set<Entity*>& entitiesInSceneGraph);
	void AddNewNodesToSceneGraph(EffectParticleEditorNode* node, Entity* effectRootNode);
	void RemoveExcessiveNodesFromSceneGraph(EffectParticleEditorNode* node, Entity* effectRootNode);

	// Move item to parent functionality.
	bool IsItemBelongToParticleEditor(GraphItem* item);

	// Whether this move is forbidden?
	bool IsMoveItemToParentForbidden(GraphItem* movedItem, GraphItem* newParentItem);

	Entity* CreateParticleEffectNode();
	
	// Get the particular type of node by the graph item. Return NULL if the graph item
	// belongs to another kind of node.
	LayerParticleEditorNode* GetLayerEditorNodeByGraphItem(GraphItem* graphItem) const;
};
	
}

#endif /* defined(__ResourceEditorQt__ParticlesEditorSceneModelHelper__) */
