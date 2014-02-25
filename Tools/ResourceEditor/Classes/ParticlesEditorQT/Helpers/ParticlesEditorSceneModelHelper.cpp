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



#include "ParticlesEditorSceneModelHelper.h"
#include "DockParticleEditor/ParticlesEditorController.h"
#include "DockSceneGraph/SceneGraphModel.h"

#include "Commands/CommandsManager.h"
#include "Commands/SceneGraphCommands.h"
#include "Commands2/ParticleEditorCommands.h"

#include "Entity/Component.h"
#include "Scene3D/Components/ParticleEffectComponent.h"

#include "../StringConstants.h"

#include "../../Qt/Scene/SceneDataManager.h"

using namespace DAVA;

// Custom processing the Selection Changed in the Scene Graph model. Returns
// TRUE if no further processing needed.
bool ParticlesEditorSceneModelHelper::ProcessSelectionChanged(const QItemSelection &selected,
                                                         const QItemSelection &deselected)
{
    int32 deselectedSize = deselected.size();
    int32 selectedSize = selected.size();

    if (selectedSize <= 0)
    {
        // De-selection happened - allow processing by caller.
        ParticlesEditorController::Instance()->CleanupSelectedNode();
        return false;
    }
    
    // Determine whether the selected node belongs to Particle Editor.
    QItemSelectionRange selectedRange = selected.at(0);
    SceneGraphItem *selectedItem = static_cast<SceneGraphItem*>(selectedRange.topLeft().internalPointer());

    if (ParticlesEditorController::Instance()->IsBelongToParticlesEditor(selectedItem))
    {
        // Our one. Select and emit the appropriate event.
        ParticlesEditorController::Instance()->SetSelectedNode(selectedItem, true);
        return !ParticlesEditorController::Instance()->ShouldDisplayPropertiesInSceneEditor(selectedItem);
    }

    // Not ours. Cleanup the selected node and return control back to the caller.
    ParticlesEditorController::Instance()->CleanupSelectedNode();
    return false;
}

// Custom node selection.
SceneGraphItem* ParticlesEditorSceneModelHelper::GetGraphItemToBeSelected(GraphItem* rootItem, Entity* node)
{
    // Do we have something marked for selection on the Particles Editor? If yes,
    // do select it.
    return GetGraphItemToBeSelectedRecursive(rootItem, node);
    
    return false;
}

void ParticlesEditorSceneModelHelper::ResetSelection()
{
    if(ParticlesEditorController::Instance())
        ParticlesEditorController::Instance()->CleanupSelectedNode();
}

SceneGraphItem* ParticlesEditorSceneModelHelper::GetGraphItemToBeSelectedRecursive(GraphItem* rootItem, Entity* node)
{
    SceneGraphItem* graphRootItem = dynamic_cast<SceneGraphItem*>(rootItem);
    if (!graphRootItem || graphRootItem->GetExtraUserData() == NULL)
    {
        return NULL;
    }
    
    BaseParticleEditorNode* editorNode = dynamic_cast<BaseParticleEditorNode*>(graphRootItem->GetExtraUserData());
    if (!editorNode)
    {
        return NULL;
    }
    
    if (editorNode->IsMarkedForSelection())
    {
        editorNode->SetMarkedToSelection(false);
        return graphRootItem;
    }
    
    // Verify all the child nodes for the Scene Graph.
    int32 childCount = rootItem->ChildrenCount();
    for (int32 i = 0; i < childCount; i ++)
    {
        SceneGraphItem* childGraphItem = dynamic_cast<SceneGraphItem*>(rootItem->Child(i));
        SceneGraphItem* graphItemToBeSelected = GetGraphItemToBeSelectedRecursive(childGraphItem, node);
        if (graphItemToBeSelected)
        {
            return graphItemToBeSelected;
        }
    }

    return NULL;
}

Entity* ParticlesEditorSceneModelHelper::PreprocessSceneNode(Entity* rawNode)
{
    // There is one and only preprocessing case - if the "raw" node is "orphaned" Particles Emitter
    // (without the ParticlesEffectNode parent), we'll create the new Particles Effect node and
    // move the raw Emitter node to it.
	ParticleEmitter * emitter = GetEmitter(rawNode);
    if (!emitter)
    {
        return rawNode;
    }

	//ParticleEmitterNode* emitterNode = static_cast<ParticleEmitterNode*>(rawNode);
	//emitterNode->SetEmitter(emitterComponent->GetParticleEmitter());
    Entity* curParentNode = rawNode->GetParent();
    ParticleEffectComponent * effectComponent = cast_if_equal<ParticleEffectComponent*>(curParentNode->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
    // If the Emitter Node has parent, but its parent is not ParticleEffectNode, we need to
    // "adopt" this node by creating ParticleEffect node and attaching.
    if (curParentNode && (effectComponent == NULL))
    {
		Entity* newParentNodeParticleEffect = CreateParticleEffectNode();
		curParentNode->AddNode(newParentNodeParticleEffect);

        // Add the emitter node to the new Effect (this will also remove it from the scene).
        newParentNodeParticleEffect->AddNode(rawNode);

        // Register the Particle Editor structure for the new node.
        EffectParticleEditorNode* effectEditorNode =
        ParticlesEditorController::Instance()->RegisterParticleEffectNode(newParentNodeParticleEffect);
        EmitterParticleEditorNode* emitterEditorNode = new EmitterParticleEditorNode(newParentNodeParticleEffect,
            rawNode, QString::fromStdString(rawNode->GetName()));
        effectEditorNode->AddNode(emitterEditorNode);

        return newParentNodeParticleEffect;
    }
    
    // No preprocessing needed.
    return rawNode;
}

Entity* ParticlesEditorSceneModelHelper::CreateParticleEffectNode()
{
	Entity * newParentNodeParticleEffect = new Entity();
	newParentNodeParticleEffect->SetName(ResourceEditor::PARTICLE_EFFECT_NODE_NAME);
	newParentNodeParticleEffect->AddComponent(ScopedPtr<ParticleEffectComponent> (new ParticleEffectComponent()));

	return newParentNodeParticleEffect;
}

bool ParticlesEditorSceneModelHelper::AddNodeToSceneGraph(SceneGraphItem *graphItem, Entity *node)
{
    ParticleEffectComponent * effectComponent = cast_if_equal<ParticleEffectComponent*>(node->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
    if (effectComponent == NULL)
    {
        // Not ours - process as-is.
        return false;
    }

    EffectParticleEditorNode* effectEditorNode = ParticlesEditorController::Instance()->GetRootForParticleEffectNode(node);
    if (!effectEditorNode)
    {
        // Possible while loading projects - register the node in this case.
        effectEditorNode = ParticlesEditorController::Instance()->RegisterParticleEffectNode(node, false);
    }
    
    if (graphItem->GetExtraUserData() == NULL)
    {
        // Register the root. Note - it has both User Data and Extra Data.
        graphItem->SetExtraUserData(effectEditorNode);
    }

    // Build the Scene Graph in a recursive way.
    BuildSceneGraphRecursive(effectEditorNode, graphItem);
    return true;
}

void ParticlesEditorSceneModelHelper::BuildSceneGraphRecursive(BaseParticleEditorNode* rootNode, SceneGraphItem* rootItem)
{
    // Prior to buildind the tree - need to check whether the children are in sync and synchronize them,
    // if not. This is actually needed for Load() process.
    SynchronizeParticleEditorTree(rootNode);
    
    // Set the UserData for inner Emitter nodes.
    if (dynamic_cast<EmitterParticleEditorNode*>(rootNode))
    {
        Entity* emitterNode = (static_cast<EmitterParticleEditorNode*>(rootNode))->GetEmitterNode();
        rootItem->SetUserData(emitterNode);
    }

    int childrenCount = rootNode->GetChildren().size();
    for (List<BaseParticleEditorNode*>::const_iterator iter = rootNode->GetChildren().begin();
         iter != rootNode->GetChildren().end(); iter ++)
    {
        BaseParticleEditorNode* childNode = (*iter);

        SceneGraphItem* childItem = new SceneGraphItem();
        
        childItem->SetExtraUserData(childNode);
        rootItem->AppendChild(childItem);

        // Repeat for all children.
        BuildSceneGraphRecursive(childNode, childItem);
    }
}

void ParticlesEditorSceneModelHelper::SynchronizeParticleEditorTree(BaseParticleEditorNode* node)
{
	EffectParticleEditorNode* effectEditorNode = dynamic_cast<EffectParticleEditorNode*>(node);
    if (effectEditorNode)
    {
		Entity* effectRootNode = node->GetRootNode();
        SynchronizeEffectParticleEditorNode(effectEditorNode, effectRootNode);
    }

    EmitterParticleEditorNode* emitterEditorNode = dynamic_cast<EmitterParticleEditorNode*>(node);
    if (emitterEditorNode)
    {
        SynchronizeEmitterParticleEditorNode(emitterEditorNode);
    }
    
    LayerParticleEditorNode* layerEditorNode = dynamic_cast<LayerParticleEditorNode*>(node);
    if (layerEditorNode)
    {
        SynchronizeLayerParticleEditorNode(layerEditorNode);
    }
}

void ParticlesEditorSceneModelHelper::SynchronizeEffectParticleEditorNode(EffectParticleEditorNode* node, Entity* effectRootNode)
{
    if (!node || !effectRootNode)
    {
        return;
    }

    // All the children of Effect Node are actually Emitters.
    int emittersCountInEffect = effectRootNode->GetChildrenCount();
    int emittersCountInEffectNode = node->GetEmittersCount();

    if (emittersCountInEffect > emittersCountInEffectNode)
    {
		AddNewNodesToSceneGraph(node, effectRootNode);
    }
	else if (emittersCountInEffectNode > emittersCountInEffect)
	{
		RemoveExcessiveNodesFromSceneGraph(node, effectRootNode);
	}
}

void ParticlesEditorSceneModelHelper::SynchronizeEmitterParticleEditorNode(EmitterParticleEditorNode* node)
{
    if (!node)
    {
        return;
    }

    ParticleEmitter* emitter = node->GetParticleEmitter();
    if (!emitter)
    {
        return;
    }

    int layersCountInEmitter = emitter->GetLayers().size();
    int layersCountInEmitterNode = node->GetLayersCount();

    if (layersCountInEmitter > 0 && layersCountInEmitterNode == 0)
    {
        // Emitter Node has no layers - need to add them.
        for (int32 i = 0; i < layersCountInEmitter; i ++)
        {
            // Create the new node and add it to the tree.
            LayerParticleEditorNode* layerNode = new LayerParticleEditorNode(node, emitter->GetLayers()[i]);
            node->AddNode(layerNode);
        }
     }
}

void ParticlesEditorSceneModelHelper::SynchronizeLayerParticleEditorNode(LayerParticleEditorNode* node)
{
    if (!node)
    {
        return;
    }
    
    ParticleEmitter* emitter = node->GetParticleEmitter();
    if (!emitter)
    {
        return;
    }

    ParticleLayer* layer = node->GetLayer();
    if (!layer)
    {
        return;
    }

	// Yuri Coder, 2012/04.29. The SuperEmitter layer might contain inner emitter.
	SynchronizeInnerEmitterNode(node);
	
    // Synchronize the Forces.
    int32 forcesCountInLayer = layer->forces.size();
    int32 forcesCountInLayerNode = node->GetForcesCount();

    if (forcesCountInLayer > 0 && forcesCountInLayerNode == 0)
    {
        // Emitter Layer has no Forces - need to add them.
        for (int32 i = 0; i < forcesCountInLayer; i ++)
        {
            ForceParticleEditorNode* forceNode = new ForceParticleEditorNode(node, i);
            node->AddNode(forceNode);
        }

        node->UpdateForcesIndices();
    }
}

void ParticlesEditorSceneModelHelper::SynchronizeInnerEmitterNode(LayerParticleEditorNode* node,
																  SceneGraphItem* layerNodeItem,
																  SceneGraphModel* sceneGraphModel)
{
	if (!node)
    {
        return;
    }
    
    ParticleLayer* layer = node->GetLayer();
    if (!layer)
    {
        return;
    }

	ParticleEmitter* innerEmitter = layer->GetInnerEmitter();
	int32 innerEmittersCountInLayerNode = node->GetInnerEmittersCount();
	
	bool needAddInnerEmitter = ((layer->type == ParticleLayer::TYPE_SUPEREMITTER_PARTICLES) &&
								innerEmitter &&
								(innerEmittersCountInLayerNode == 0));
	
	if (needAddInnerEmitter)
	{
		// There is inner emitter defined, but it doesn't exist in the Layer.
		// Need to add it.
		InnerEmitterParticleEditorNode* innerEmitterEditorNode = new InnerEmitterParticleEditorNode(node);
		
		node->AddNode(innerEmitterEditorNode);

		// Also update the Scene Graph, if requested.
		if (layerNodeItem && sceneGraphModel)
		{
			SceneGraphItem* childItem = new SceneGraphItem();
			childItem->SetExtraUserData(innerEmitterEditorNode);
			sceneGraphModel->AddNodeToTree(layerNodeItem, childItem);
		}

		return;
	}

	bool needRemoveInnerEmitter = ((layer->type != ParticleLayer::TYPE_SUPEREMITTER_PARTICLES &&
									innerEmittersCountInLayerNode > 0));
	if (needRemoveInnerEmitter)
	{
		// Update the Scene Graph, if needed.
		if (layerNodeItem && sceneGraphModel)
		{
			int childrenCount = layerNodeItem->ChildrenCount();
			List<GraphItem*> childItemsToRemove;

			// Two-step pass is needed - first one is to find all the children to remove,
			// second one is to do actual remove.
			for (int i = 0; i < childrenCount; i ++)
			{
				GraphItem* childItem = layerNodeItem->Child(i);
				SceneGraphItem* childSceneGraphItem = dynamic_cast<SceneGraphItem*>(childItem);
				ExtraUserData* extraUserData = childSceneGraphItem->GetExtraUserData();

				if (childSceneGraphItem && extraUserData &&
					dynamic_cast<InnerEmitterParticleEditorNode*>(extraUserData))
				{
					childItemsToRemove.push_back(childItem);
				}
			}

			for (List<GraphItem*>::iterator iter = childItemsToRemove.begin();
				 iter != childItemsToRemove.end(); iter ++)
			{
				GraphItem* childItem = (*iter);
				sceneGraphModel->RemoveNodeFromTree(layerNodeItem, childItem);
			}
		}

		// AFTER the Scene Graph was updated - do the actual remove.
		BaseParticleEditorNode::PARTICLEEDITORNODESLIST nodesToRemove;
		for (BaseParticleEditorNode::PARTICLEEDITORNODESLIST::const_iterator iter = node->GetChildren().begin();
			 iter != node->GetChildren().end(); iter ++)
		{
			if (dynamic_cast<InnerEmitterParticleEditorNode*>(*iter))
			{
				nodesToRemove.push_back(*iter);
			}
		}
		
		for (BaseParticleEditorNode::PARTICLEEDITORNODESLIST::iterator removeIter = nodesToRemove.begin();
			 removeIter != nodesToRemove.end(); removeIter ++)
		{
			node->RemoveNode(*removeIter);
		}
	}
}

bool ParticlesEditorSceneModelHelper::NeedDisplaySceneEditorPopupMenuItems(const QModelIndex &index) const
{
    ExtraUserData* extraUserData = GetExtraUserDataByModelIndex(index);
    if (!extraUserData)
    {
        return true;
    }
    
    // We should block Scene Editor popup menu items for all child nodes for Particle Effect Node.
    if (dynamic_cast<EmitterParticleEditorNode*>(extraUserData))
    {
        return false;
    }

    if (dynamic_cast<LayerParticleEditorNode*>(extraUserData))
    {
        return false;
    }

    if (dynamic_cast<ForceParticleEditorNode*>(extraUserData))
    {
        return false;
    }

	if (dynamic_cast<InnerEmitterParticleEditorNode*>(extraUserData))
    {
        return false;
    }

    return true;
}

ExtraUserData* ParticlesEditorSceneModelHelper::GetExtraUserDataByModelIndex(const QModelIndex& modelIndex) const
{
    if (!modelIndex.isValid())
    {
        return NULL;
    }

    SceneGraphItem *item = static_cast<SceneGraphItem*>(modelIndex.internalPointer());
    if (!item)
    {
        return NULL;
    }
    
    return item->GetExtraUserData();
}

ExtraUserData* ParticlesEditorSceneModelHelper::GetExtraUserData(GraphItem* item) const
{
	if (!item || !dynamic_cast<SceneGraphItem*>(item))
	{
		return false;
	}
	
	return (static_cast<SceneGraphItem*>(item))->GetExtraUserData();
}

void ParticlesEditorSceneModelHelper::AddPopupMenuItems(QMenu& menu, const QModelIndex &index) const
{
    ExtraUserData* extraUserData = GetExtraUserDataByModelIndex(index);
    if (!extraUserData)
    {
        return;
    }
    
    // Which kind of Node is it?
    if (dynamic_cast<EffectParticleEditorNode*>(extraUserData))
    {
		// Yuri Coder, 2013/07/24. These commands aren't used now and to be removed soon.
		/*
        // Effect Node. Allow to add Particle Emitters and start/stop the animation.
		QMenu* emittersMenu = menu.addMenu("Particle Effect");
        AddActionToMenu(emittersMenu, QString("Add Particle Emitter"), new CommandAddParticleEmitter());
        AddActionToMenu(emittersMenu, QString("Start Particle Effect"), new CommandStartStopParticleEffect(true));
        AddActionToMenu(emittersMenu, QString("Stop Particle Effect"), new CommandStartStopParticleEffect(false));
        AddActionToMenu(emittersMenu, QString("Restart Particle Effect"), new CommandRestartParticleEffect());
		 */
    }
    else if (dynamic_cast<EmitterParticleEditorNode*>(extraUserData))
    {
		// Yuri Coder, 2013/07/24. These commands aren't used now and to be removed soon.
		/*
        // For Particle Emitter we also allow to load/save it.
        AddActionToMenu(&menu, QString("Load Emitter from Yaml"), new CommandLoadParticleEmitterFromYaml());
        AddActionToMenu(&menu, QString("Save Emitter to Yaml"), new CommandSaveParticleEmitterToYaml(false));
        AddActionToMenu(&menu, QString("Save Emitter to Yaml As"), new CommandSaveParticleEmitterToYaml(true));
        
        // Emitter node. Allow to remove it and also add Layers.
        AddActionToMenu(&menu, QString("Remove Particle Emitter"), new CommandRemoveSceneNode());
        AddActionToMenu(&menu, QString("Add Layer"), new CommandAddParticleEmitterLayer());
		*/
    }
    else if (dynamic_cast<LayerParticleEditorNode*>(extraUserData))
    {
		// Yuri Coder, 2013/07/24. These commands aren't used now and to be removed soon.
		/*
        // Layer Node. Allow to remove it and add new Force.
        AddActionToMenu(&menu, QString("Remove Layer"), new CommandRemoveParticleEmitterLayer());
		
        AddActionToMenu(&menu, QString("Clone Layer"), new CommandCloneParticleEmitterLayer());
        AddActionToMenu(&menu, QString("Add Force"), new CommandAddParticleEmitterForce());
		 */
    }
    else if (dynamic_cast<ForceParticleEditorNode*>(extraUserData))
    {
		// Yuri Coder, 2013/07/24. These commands aren't used now and to be removed soon.
		/*
        // Force Node. Allow to remove it.
        AddActionToMenu(&menu, QString("Remove Force"), new CommandRemoveParticleEmitterForce());
		 */
    }
	else if (dynamic_cast<InnerEmitterParticleEditorNode*>(extraUserData))
	{
		/*
		// TODO!!! YURI CODER, 2013/07/22. TEMPORARILY commented out!!!
		// Inner Emitter node, similar to Emitter one.
		AddActionToMenu(&menu, QString("Load Emitter from Yaml"), new CommandLoadInnerEmitterFromYaml());
        AddActionToMenu(&menu, QString("Save Emitter to Yaml"), new CommandSaveInnerEmitterToYaml(false));
        AddActionToMenu(&menu, QString("Save Emitter to Yaml As"), new CommandSaveInnerEmitterToYaml(true));
		 */
	}
}

void ParticlesEditorSceneModelHelper::AddActionToMenu(QMenu *menu, const QString &actionTitle, Command2 *command) const
{
    QAction *action = menu->addAction(actionTitle);
    //action->setData(PointerHolder<Command2 *>::ToQVariant(command));
}

bool ParticlesEditorSceneModelHelper::NeedMoveItemToParent(GraphItem* movedItem, GraphItem* newParentItem)
{
	ExtraUserData* movedItemUserData = (static_cast<SceneGraphItem*>(movedItem))->GetExtraUserData();
	ExtraUserData* newParentItemUserData = (static_cast<SceneGraphItem*>(newParentItem))->GetExtraUserData();
	
	if (!movedItemUserData || !newParentItemUserData)
	{
		return false;
	}
	
	// Yuri Coder, 2013/14/01. The first case - if we are moving Layers.
	if (dynamic_cast<LayerParticleEditorNode*>(movedItemUserData) && dynamic_cast<LayerParticleEditorNode*>(newParentItemUserData))
	{
		return true;
	}

	// The second case - if we are moving Layer to particular Emitter.
	if (dynamic_cast<LayerParticleEditorNode*>(movedItemUserData) && dynamic_cast<EmitterParticleEditorNode*>(newParentItemUserData))
	{
		return true;
	}

	// Moving Emitters between different Effect Nodes should also be handled separately.
	if (dynamic_cast<EmitterParticleEditorNode*>(movedItemUserData) && dynamic_cast<EffectParticleEditorNode*>(newParentItemUserData))
	{
		return true;
	}

	// Moving Emitters inside the same Effect Node is also allowed.
	if (dynamic_cast<EmitterParticleEditorNode*>(movedItemUserData) && dynamic_cast<EmitterParticleEditorNode*>(newParentItemUserData))
	{
		return true;
	}

	// The move should be separately handled in the following cases:
	// The Moved Item is Layer or Force.
	// The New Parent Item is Layer or Force.
	if (IsItemBelongToParticleEditor(movedItem) || IsItemBelongToParticleEditor(newParentItem))
	{
		return true;
	}

	// Some move combinations can be forbidden at all, but must be processed on this level.
	if (IsMoveItemToParentForbidden(movedItem, newParentItem))
	{
		return true;
	}
	
	return false;
}

bool ParticlesEditorSceneModelHelper::MoveItemToParent(GraphItem* movedItem, GraphItem* newParentItem)
{
	ExtraUserData* movedItemUserData = (static_cast<SceneGraphItem*>(movedItem))->GetExtraUserData();
	ExtraUserData* newParentItemUserData = (static_cast<SceneGraphItem*>(newParentItem))->GetExtraUserData();
	
	if (!movedItemUserData || !newParentItemUserData)
	{
		return false;
	}
	
	// Yuri Coder, 2013/14/01. The first case - if we are moving Layers.
	LayerParticleEditorNode* movedItemNode = dynamic_cast<LayerParticleEditorNode*>(movedItemUserData);
	LayerParticleEditorNode* newLayerParentNode = dynamic_cast<LayerParticleEditorNode*>(newParentItemUserData);
	
	if (movedItemNode && newLayerParentNode)
	{
		// We are moving Layers from one emitter to another one (or changing its order
		// on the same emitter's level).
		return ParticlesEditorController::Instance()->MoveLayer(movedItemNode, newLayerParentNode);
	}

	EmitterParticleEditorNode* newEmitterParentNode = dynamic_cast<EmitterParticleEditorNode*>(newParentItemUserData);
	if (movedItemNode && newEmitterParentNode)
	{
		// We are moving Layers between emitters.
		return ParticlesEditorController::Instance()->MoveLayer(movedItemNode, newEmitterParentNode);
	}

	EmitterParticleEditorNode* movedItemEmitterNode = dynamic_cast<EmitterParticleEditorNode*>(movedItemUserData);
	EffectParticleEditorNode* newEffectParentNode = dynamic_cast<EffectParticleEditorNode*>(newParentItemUserData);
	if (movedItemEmitterNode && newEffectParentNode)
	{
		// We are moving Emitter from one effect to another one.
		return ParticlesEditorController::Instance()->MoveEmitter(movedItemEmitterNode, newEffectParentNode, NULL);
	}

	newEmitterParentNode = dynamic_cast<EmitterParticleEditorNode*>(newParentItemUserData);
	if (movedItemEmitterNode && newEmitterParentNode)
	{
		// We are moving Emitters within the same Effect or between Effects.
		return ParticlesEditorController::Instance()->MoveEmitter(movedItemEmitterNode, newEmitterParentNode);
	}

	if (IsMoveItemToParentForbidden(movedItem, newParentItem))
	{
		// The move is forbidden, no update required.
		return false;
	}

	// Yuri Coder, 2013/01/11. Currently moving of "internal" Particle Editor items isn't supported.
	// TODO: return to this functionality when the other issues will be completed.
	return false;
}

bool ParticlesEditorSceneModelHelper::IsItemBelongToParticleEditor(GraphItem* item)
{
	if (!item || !dynamic_cast<SceneGraphItem*>(item))
	{
		return false;
	}

	ExtraUserData* extraUserData = (static_cast<SceneGraphItem*>(item))->GetExtraUserData();
    if (!extraUserData)
    {
        return false;
    }

	// Which kind of Node is it?
	if (dynamic_cast<LayerParticleEditorNode*>(extraUserData) ||
		dynamic_cast<ForceParticleEditorNode*>(extraUserData))
	{
		return true;
	}
	
	return false;
}

bool ParticlesEditorSceneModelHelper::IsMoveItemToParentForbidden(GraphItem* movedItem, GraphItem* newParentItem)
{
	ExtraUserData* movedItemData = GetExtraUserData(movedItem);
	ExtraUserData* newParentItemData = GetExtraUserData(newParentItem);
	if (!movedItemData || !newParentItemData)
	{
		return false;
	}
	
	// The following situations are forbidden:
	// The Particle Effect node is moved to other Particle Effect.
	// The Particle Effect node is moved to Particle Emitter (vise versa is possible though).
	if (dynamic_cast<EffectParticleEditorNode*>(movedItemData) &&
		dynamic_cast<EffectParticleEditorNode*>(newParentItemData))
	{
		return true;
	}

	if (dynamic_cast<EffectParticleEditorNode*>(movedItemData) &&
		dynamic_cast<EmitterParticleEditorNode*>(newParentItemData))
	{
		return true;
	}


	return false;
}

SceneGraphItem* ParticlesEditorSceneModelHelper::GetGraphItemForParticlesLayer(GraphItem* rootItem,
																			   DAVA::ParticleLayer* layer)
{
	// Check fot the current root
	SceneGraphItem* curItem = dynamic_cast<SceneGraphItem*>(rootItem);
	if (curItem && curItem->GetExtraUserData())
	{
		LayerParticleEditorNode* editorNode = dynamic_cast<LayerParticleEditorNode*>(curItem->GetExtraUserData());
		if (editorNode && editorNode->GetLayer() == layer)
		{
			return curItem;
		}
	}
	
	// Verify for all children.
	int32 childrenCount = rootItem->ChildrenCount();
	for (int32 i = 0; i < childrenCount; i ++)
	{
		SceneGraphItem* curItem = dynamic_cast<SceneGraphItem*>(rootItem->Child(i));
		SceneGraphItem* resultItem = GetGraphItemForParticlesLayer(curItem, layer);
		if (resultItem)
		{
			return resultItem;
		}
	}
	
	// Nothing is found...
	return NULL;
}

void* ParticlesEditorSceneModelHelper::GetPersistentDataForModelIndex(const QModelIndex& modelIndex) const
{
	ExtraUserData* extraData = GetExtraUserDataByModelIndex(modelIndex);
	if (!extraData)
	{
		// Not ours..
		return NULL;
	}
	
	return extraData;
}

bool ParticlesEditorSceneModelHelper::IsGraphItemCheckable(GraphItem* graphItem) const
{
	// Only Particle Editor Layers are checkable for now.
	return (GetLayerEditorNodeByGraphItem(graphItem) != NULL);
}

bool ParticlesEditorSceneModelHelper::GetCheckableStateForGraphItem(GraphItem* graphItem) const
{
	LayerParticleEditorNode* layerEditorNode = GetLayerEditorNodeByGraphItem(graphItem);
	if (!layerEditorNode || !layerEditorNode->GetLayer())
	{
		return false;
	}
	
	return !layerEditorNode->GetLayer()->GetDisabled();
}

void ParticlesEditorSceneModelHelper::SetCheckableStateForGraphItem(GraphItem* graphItem, bool value)
{
	LayerParticleEditorNode* layerEditorNode = GetLayerEditorNodeByGraphItem(graphItem);
	if (!layerEditorNode || !layerEditorNode->GetLayer())
	{
		return;
	}
	
	// Execute the appropriate command.
	/*
	CommandsManager::Instance()->ExecuteAndRelease(new CommandUpdateParticleLayerEnabled(layerEditorNode->GetLayer(), value),
												   SceneDataManager::Instance()->SceneGetActive()->GetScene());
	 */
}

LayerParticleEditorNode* ParticlesEditorSceneModelHelper::GetLayerEditorNodeByGraphItem(GraphItem* graphItem) const
{
	SceneGraphItem* curItem = dynamic_cast<SceneGraphItem*>(graphItem);
	if (curItem && curItem->GetExtraUserData())
	{
		LayerParticleEditorNode* editorNode = dynamic_cast<LayerParticleEditorNode*>(curItem->GetExtraUserData());
		return editorNode;
	}
	
	return NULL;
}

void ParticlesEditorSceneModelHelper::BuildEntitiesSets(EffectParticleEditorNode* node, Entity* effectRootNode,
														Set<Entity*>& entitiesInParticleEditor,
														Set<Entity*>& entitiesInSceneGraph)
{
	for (List<BaseParticleEditorNode*>::const_iterator iter = node->GetChildren().begin();
		 iter != node->GetChildren().end(); iter ++)
	{
		EmitterParticleEditorNode* childNode = dynamic_cast<EmitterParticleEditorNode*>(*iter);
		if (!childNode || !childNode->GetEmitterNode())
		{
			continue;
		}
		
		entitiesInParticleEditor.insert(childNode->GetEmitterNode());
	}
	
	// Build the set of nodes present
    int emittersCountInEffect = effectRootNode->GetChildrenCount();
	for (int32 i = 0; i < emittersCountInEffect; i ++)
	{
		// Create the new Emitter and add it to the tree.
		Entity* effectEntity = effectRootNode->GetChild(i);
		if (!effectEntity)
		{
			continue;
		}
		
		entitiesInSceneGraph.insert(effectEntity);
	}

}

void ParticlesEditorSceneModelHelper::AddNewNodesToSceneGraph(EffectParticleEditorNode* node, Entity* effectRootNode)
{
	Set<Entity*> entitiesInParticleEditor;
	Set<Entity*> entitiesInSceneGraph;
	BuildEntitiesSets(node, effectRootNode, entitiesInParticleEditor, entitiesInSceneGraph);
	
	// Scene Graph contains more items than Particle Emitter node, add the new ones.
    int emittersCountInEffect = effectRootNode->GetChildrenCount();
	for (int32 i = 0; i < emittersCountInEffect; i ++)
	{
		if (entitiesInParticleEditor.find(effectRootNode->GetChild(i)) != entitiesInParticleEditor.end())
		{
			continue;
		}

		// Create the new Emitter and add it to the tree.
		ParticleEmitter * emitter =  GetEmitter(effectRootNode->GetChild(i));
		if (!emitter)
		{
			continue;
		}
		
		EmitterParticleEditorNode* emitterEditorNode = new EmitterParticleEditorNode(effectRootNode, effectRootNode->GetChild(i),
																					 QString::fromStdString(effectRootNode->GetChild(i)->GetName()));
		node->AddNode(emitterEditorNode);
	}
}

void ParticlesEditorSceneModelHelper::RemoveExcessiveNodesFromSceneGraph(EffectParticleEditorNode* node, Entity* effectRootNode)
{
	// Some Particle Emitters were removed, update the tree.
	Set<Entity*> entitiesInParticleEditor;
	Set<Entity*> entitiesInSceneGraph;
	BuildEntitiesSets(node, effectRootNode, entitiesInParticleEditor, entitiesInSceneGraph);
	
	// Now compare these two lists and remove the excessive ones.
	for (Set<Entity*>::iterator iter = entitiesInParticleEditor.begin(); iter != entitiesInParticleEditor.end();
		 iter ++)
	{
		if (entitiesInSceneGraph.find(*iter) == entitiesInSceneGraph.end())
		{
			ParticlesEditorController::Instance()->RemoveParticleEmitterNode(*iter);
		}
	}
}

void ParticlesEditorSceneModelHelper::UpdateLayerRepresentation(GraphItem* rootItem,
																DAVA::ParticleLayer* layer,
																SceneGraphModel* sceneGraphModel)
{
	// Get the appropriate Scene Graph Item.
	SceneGraphItem* layerItem = GetGraphItemForParticlesLayer(rootItem, layer);
	if (!layerItem)
	{
		return;
	}
	
	// Extract the Particle Editor Node from it...
	LayerParticleEditorNode* layerNode = GetLayerEditorNodeByGraphItem(layerItem);
	if (!layerNode)
	{
		return;
	}

	bool innerEmitterExistedBeforeSync = (layerNode->GetInnerEmittersCount() > 0);
	
	// ... and do the synchronization for inner emitters.
	SynchronizeInnerEmitterNode(layerNode, layerItem, sceneGraphModel);
}
