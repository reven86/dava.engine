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


#include "ItemsCommand.h"
#include "HierarchyTreeController.h"
#include "ScreenWrapper.h"
#include "CopyPasteHelper.h"

UndoableHierarchyTreeNodeCommand::UndoableHierarchyTreeNodeCommand()
{
	this->redoNode = NULL;
	this->type = TYPE_UNDEFINED;
}

void UndoableHierarchyTreeNodeCommand::SetRedoNode(HierarchyTreeNode* redoNode)
{
	this->redoNode = redoNode;
}

void UndoableHierarchyTreeNodeCommand::ReturnRedoNodeToScene()
{
	if (this->redoNode)
	{
		// Need to recover the node previously deleted.
		HierarchyTreeController::Instance()->ReturnNodeToScene(redoNode);
	}
}

void UndoableHierarchyTreeNodeCommand::PrepareRemoveFromSceneInformation()
{
	if (this->redoNode)
	{
		this->redoNode->PrepareRemoveFromSceneInformation();
	}
}

void UndoableHierarchyTreeNodeCommand::DetectType(HierarchyTreeNode* node)
{
	if (dynamic_cast<HierarchyTreePlatformNode*>(node))
	{
		this->type = TYPE_PLATFORM;
	}
	else if (dynamic_cast<HierarchyTreeAggregatorNode*>(node))
	{
		this->type = TYPE_AGGREGATOR;
	}
	else if (dynamic_cast<HierarchyTreeScreenNode*>(node))
	{
		this->type = TYPE_SCREEN;
	}
	else
	{
		this->type = TYPE_CONTROLS;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

CreatePlatformCommand::CreatePlatformCommand(const QString& name, const Vector2& size) :
	UndoableHierarchyTreeNodeCommand()
{
	this->name = name;
	this->size = size;
}

void CreatePlatformCommand::Execute()
{
	if (this->redoNode == NULL)
	{
		SetRedoNode(HierarchyTreeController::Instance()->AddPlatform(name, size));
		redoNode->SetMarked(true);
	}
	else
	{
		ReturnRedoNodeToScene();
	}
}

void CreatePlatformCommand::Rollback()
{
	PrepareRemoveFromSceneInformation();
	
	if (this->redoNode)
	{
		// Remove the created node from the scene, but keep it in memory.
		HierarchyTreeController::Instance()->DeleteNode(this->redoNode->GetId(), false, true, true);
	}
}

void CreatePlatformCommand::IncrementUnsavedChanges()
{
	this->redoNode->GetParent()->IncrementUnsavedChanges();
}

void CreatePlatformCommand::DecrementUnsavedChanges()
{
	this->redoNode->GetParent()->DecrementUnsavedChanges();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

CreateScreenCommand::CreateScreenCommand(const QString& name, HierarchyTreeNode::HIERARCHYTREENODEID platformId) :
	UndoableHierarchyTreeNodeCommand()
{
	this->name = name;
	this->platformId = platformId;
}

void CreateScreenCommand::Execute()
{
	if (this->redoNode == NULL)
	{
		SetRedoNode(HierarchyTreeController::Instance()->AddScreen(name, platformId));
		redoNode->SetMarked(true);
	}
	else
	{
		ReturnRedoNodeToScene();
	}
}

void CreateScreenCommand::Rollback()
{
	PrepareRemoveFromSceneInformation();
	
	if (this->redoNode)
	{
		// Remove the created node from the scene, but keep it in memory.
		HierarchyTreeController::Instance()->DeleteNode(this->redoNode->GetId(), false, true, true);
	}
}

void CreateScreenCommand::IncrementUnsavedChanges()
{
	redoNode->GetParent()->IncrementUnsavedChanges();
}

void CreateScreenCommand::DecrementUnsavedChanges()
{
	redoNode->GetParent()->DecrementUnsavedChanges();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

CreateAggregatorCommand::CreateAggregatorCommand(const QString& name, HierarchyTreeNode::HIERARCHYTREENODEID platformId, const Rect& rect) :
UndoableHierarchyTreeNodeCommand()
{
	this->name = name;
	this->platformId = platformId;
	this->rect = rect;
}

void CreateAggregatorCommand::Execute()
{
	if (this->redoNode == NULL)
	{
		SetRedoNode(HierarchyTreeController::Instance()->AddAggregator(name, platformId, rect));
		redoNode->SetMarked(true);
	}
	else
	{
		ReturnRedoNodeToScene();
	}
}

void CreateAggregatorCommand::Rollback()
{
	PrepareRemoveFromSceneInformation();
	
	if (this->redoNode)
	{
		// Remove the created node from the scene, but keep it in memory.
		HierarchyTreeController::Instance()->DeleteNode(this->redoNode->GetId(), false, true, true);
	}
}

void CreateAggregatorCommand::IncrementUnsavedChanges()
{
	redoNode->GetParent()->IncrementUnsavedChanges();
}

void CreateAggregatorCommand::DecrementUnsavedChanges()
{
	redoNode->GetParent()->DecrementUnsavedChanges();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

CreateControlCommand::CreateControlCommand(HierarchyTreeNode::HIERARCHYTREENODEID typeId, const QPoint& pos)
{
    this->typeId = typeId;
	this->pos = pos;
	this->createdControlID = HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;

	this->commandMode = CREATE_FROM_POINT;
	this->parentNode = NULL;
	this->redoNode = NULL;
    this->insertAfterNode = NULL;
}

CreateControlCommand::CreateControlCommand(HierarchyTreeNode::HIERARCHYTREENODEID typeId, HierarchyTreeNode* parent, HierarchyTreeNode* insertAfter/*=NULL*/)
{
    this->typeId = typeId;
	this->pos = pos;
	this->createdControlID = HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;
	
	this->commandMode = CREATE_FROM_NODE;
	this->parentNode = parent;
	this->redoNode = NULL;
    this->insertAfterNode = insertAfter;
}

void CreateControlCommand::Execute()
{
	if (this->redoNode)
	{
		// Need to recover the node previously deleted.
		HierarchyTreeController::Instance()->ReturnNodeToScene(redoNode);
		return;
	}

	
	// The command is executed for the first time; create the node.
	HierarchyTreeNode::HIERARCHYTREENODEID newControlID = HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;
	switch (this->commandMode)
	{
		case CREATE_FROM_POINT:
		{
			// Use the point passed.
			newControlID = HierarchyTreeController::Instance()->CreateNewControl(typeId, pos);
			break;
		}

		case CREATE_FROM_NODE:
		{
			static const Vector2 NEW_CONTROL_OFFSET = Vector2(10, 10);
			DVASSERT(parentNode);
			newControlID = HierarchyTreeController::Instance()->CreateNewControl(typeId, NEW_CONTROL_OFFSET, this->parentNode);
			break;
		}

		default:
		{
			break;
		}
	}

	if (newControlID == HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY)
	{
		// The control wasn't created.
		return;
	}
	
    if(insertAfterNode)
    {
        HierarchyTreeNode* node = HierarchyTreeController::Instance()->GetTree().GetNode(newControlID);
        node->SetParent(parentNode, insertAfterNode);
        HierarchyTreeController::Instance()->EmitHierarchyTreeUpdated();
    }
	this->createdControlID = newControlID;
}

void CreateControlCommand::Rollback()
{
	if (this->createdControlID == HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY)
	{
		// The control wasn't created yet or error happened.
		return;
	}

	PrepareRedoInformation();
	
	// OK, we have all the information we need to perform Redo. Remove the created node
	// from the scene, but keep it in memory.
	HierarchyTreeController::Instance()->DeleteNode(createdControlID, false, true, false);
}

void CreateControlCommand::PrepareRedoInformation()
{
	// Clone the current control node, remember the pointer to the previous node in the list
	// to restore the position of the node removed in case of Redo.
	HierarchyTreeNode* createdNode = HierarchyTreeController::Instance()->GetTree().GetNode(this->createdControlID);
	if (!createdNode || !createdNode->GetParent())
	{
		this->redoNode = NULL;
		return;
	}
	
	createdNode->PrepareRemoveFromSceneInformation();
	this->redoNode = createdNode;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

DeleteSelectedNodeCommand::DeleteSelectedNodeCommand(const HierarchyTreeNode::HIERARCHYTREENODESLIST& nodes, bool needDeleteFiles)
{
	this->nodes = nodes;
	this->deleteFiles = needDeleteFiles;

	DVASSERT(nodes.empty() == false);
	HierarchyTreeNode* itemNode = nodes.front();

	DetectType(itemNode);

	if (type == TYPE_AGGREGATOR)
	{
		StoreParentsOfRemovingAggregatorControls(nodes);
	}

	parentId = itemNode->GetParent()->GetId();
}

void DeleteSelectedNodeCommand::Execute()
{
	if (this->redoNodes.size() == 0)
	{
		// Prepare the Redo information for the first time.
		PrepareRedoInformation();
	}

	// Delete the node from scene, but keep in memory.
	HierarchyTreeController::Instance()->DeleteNodes(this->nodes, false, true, this->deleteFiles);
}

void DeleteSelectedNodeCommand::StoreParentsOfRemovingAggregatorControls(const HierarchyTreeNode::HIERARCHYTREENODESLIST& nodes)
{
	HierarchyTreeNode::HIERARCHYTREENODESCONSTITER it;
	for (it = nodes.begin(); it != nodes.end(); ++it)
	{
		if (!dynamic_cast<HierarchyTreeAggregatorNode*>(*it))
		{
			HierarchyTreeScreenNode* screen = HierarchyTreeController::Instance()->GetScreenNodeForNode(*it);

			if (screen)
			{
				parentsOfRemovingAggregatorControls.insert(screen->GetId());
			}
		}
	}
}

void DeleteSelectedNodeCommand::PrepareRedoInformation()
{
	// Remember the nodes which will be removed from the scene.
	for (HierarchyTreeNode::HIERARCHYTREENODESLIST::iterator iter = this->nodes.begin();
		 iter != this->nodes.end(); iter ++)
	{
		HierarchyTreeNode* nodeToRedo = (*iter);
		if (!nodeToRedo || !nodeToRedo->GetParent())
		{
			continue;
		}

		nodeToRedo->PrepareRemoveFromSceneInformation();
		this->redoNodes.push_back(nodeToRedo);
	}
}

void DeleteSelectedNodeCommand::Rollback()
{
	if (redoNodes.size() == 0)
	{
		return;
	}

	HierarchyTreeController::Instance()->ReturnNodeToScene(redoNodes);
}

void DeleteSelectedNodeCommand::IncrementUnsavedChanges()
{
	switch (type)
	{
		case TYPE_PLATFORM:
		case TYPE_SCREEN:
			HierarchyTreeController::Instance()->GetTree().GetNode((parentId))->IncrementUnsavedChanges();
			break;

		case TYPE_AGGREGATOR:
			HierarchyTreeController::Instance()->GetTree().GetNode((parentId))->IncrementUnsavedChanges();
			for (Set<HierarchyTreeNode::HIERARCHYTREENODEID>::iterator it = parentsOfRemovingAggregatorControls.begin();
				 it != parentsOfRemovingAggregatorControls.end();
				 ++it)
			{
				HierarchyTreeNode* node = HierarchyTreeController::Instance()->GetTree().GetNode(*it);
				if (node)
				{
					node->IncrementUnsavedChanges();
				}
			}
			break;

		case TYPE_CONTROLS:
		default:
			BaseCommand::IncrementUnsavedChanges();
			break;
	}
}

void DeleteSelectedNodeCommand::DecrementUnsavedChanges()
{
	switch (type)
	{
		case TYPE_PLATFORM:
		case TYPE_SCREEN:
			HierarchyTreeController::Instance()->GetTree().GetNode((parentId))->DecrementUnsavedChanges();
			break;

		case TYPE_AGGREGATOR:
			HierarchyTreeController::Instance()->GetTree().GetNode((parentId))->DecrementUnsavedChanges();
			for (Set<HierarchyTreeNode::HIERARCHYTREENODEID>::iterator it = parentsOfRemovingAggregatorControls.begin();
				 it != parentsOfRemovingAggregatorControls.end();
				 ++it)
			{
				HierarchyTreeNode* node = HierarchyTreeController::Instance()->GetTree().GetNode(*it);
				if (node)
				{
					node->DecrementUnsavedChanges();
				}
			}
			break;

		case TYPE_CONTROLS:
		default:
			BaseCommand::DecrementUnsavedChanges();
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

ChangeNodeHeirarchy::ChangeNodeHeirarchy(HierarchyTreeNode::HIERARCHYTREENODEID targetNodeID, HierarchyTreeNode::HIERARCHYTREENODEID afterNodeID, HierarchyTreeNode::HIERARCHYTREENODESIDLIST items)
{
	this->targetNodeID = targetNodeID;
	this->afterNodeID = afterNodeID;
	this->items = items;

	DVASSERT(items.empty() == false);
	HierarchyTreeNode* itemNode = HierarchyTreeController::Instance()->GetTree().GetNode((items.front()));
	DetectType(itemNode);

	// Remember the previous parent IDs for the commands. Note - we cannot store just pointers
	// to the parents since they may gone.
	StorePreviousParents();
}

void ChangeNodeHeirarchy::StorePreviousParents()
{
	for (HierarchyTreeNode::HIERARCHYTREENODESIDLIST::iterator iter = items.begin();
		 iter != items.end();
		 ++iter)
	{
		HierarchyTreeNode* node = HierarchyTreeController::Instance()->GetTree().GetNode((*iter));
		if (!node)
		{
			continue;
		}
		
		HierarchyTreeNode* parentNode = node->GetParent();
		if (!parentNode)
		{
			continue;
		}
		
		HierarchyTreeNode::HIERARCHYTREENODEID addAfter = HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;
		HierarchyTreeNode::HIERARCHYTREENODEID lastId = HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;
		const HierarchyTreeNode::HIERARCHYTREENODESLIST& childs = parentNode->GetChildNodes();
		for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator citer = childs.begin();
			 citer != childs.end();
			 ++citer)
		{
			if (node == (*citer))
				addAfter = lastId;
			lastId = (*citer)->GetId();
		}
		
		// The Previous Parents are stored in the "item ID - parent ID" map.
		this->previousParents.insert(std::make_pair(*iter, PreviousState(parentNode->GetId(), addAfter, node->GetName())));
	}
}

void ChangeNodeHeirarchy::Execute()
{
	HierarchyTreeNode* targetNode = HierarchyTreeController::Instance()->GetTree().GetNode(targetNodeID);
	HierarchyTreeNode* insertAfterNode = HierarchyTreeController::Instance()->GetTree().GetNode(afterNodeID);
	if (!targetNode)
	{
		// Possible in Redo case if some changes in tree were made.
		return;
	}

	
	for (HierarchyTreeNode::HIERARCHYTREENODESIDLIST::iterator iter = items.begin();
		 iter != items.end();
		 ++iter)
	{
		HierarchyTreeNode* node = HierarchyTreeController::Instance()->GetTree().GetNode((*iter));
		if (node)
		{
			if (dynamic_cast<HierarchyTreeScreenNode*>(node))
			{
				HierarchyTreeNode* sourceNode = node->GetParent();

				if (sourceNode != targetNode)
				{
					HierarchyTreeNode::HIERARCHYTREENODESLIST screens;
					screens.push_back(node);
					HierarchyTreeController::Instance()->DeleteNodesFiles(screens);
					
					// If we insert aggregators or screens into platform - check inserted items name. We should not
					// insert items with existing names.
					HierarchyTreePlatformNode* targetPlatform = dynamic_cast<HierarchyTreePlatformNode*>(targetNode);
					if (targetPlatform)
					{
						if (targetPlatform->IsAggregatorOrScreenNamePresent(node->GetName()))
						{
							QString newName = CopyPasteHelper::FormatCopyName(node->GetName(), targetNode);
							node->SetName(newName);
						}
					}
				}
			}

			//YZ backlight parent rect
			bool isNodeSelected = false;
			HierarchyTreeControlNode* controlNode = dynamic_cast<HierarchyTreeControlNode*>(node);
			if (controlNode)
			{
				isNodeSelected = HierarchyTreeController::Instance()->IsControlSelected(controlNode);
				HierarchyTreeController::Instance()->UnselectControl(controlNode);
                CopyPasteHelper::UpdateAggregators(controlNode, targetNode);
			}

			node->SetParent(targetNode, insertAfterNode);
			//insertAfterNode = node;
			
			if (isNodeSelected)
			{
				HierarchyTreeController::Instance()->SelectControl(controlNode);
			}
		}
	}
	
	HierarchyTreeController::Instance()->EmitHierarchyTreeUpdated(false);
	HierarchyTreeController::Instance()->ResetSelectedControl();
}

void ChangeNodeHeirarchy::Rollback()
{
	for (PARENTNODESMAPITER iter = previousParents.begin(); iter != previousParents.end(); iter ++)
	{
		HierarchyTreeNode* currentNode = HierarchyTreeController::Instance()->GetTree().GetNode(iter->first);
		HierarchyTreeNode* prevParentNode = HierarchyTreeController::Instance()->GetTree().GetNode(iter->second.parent);
		HierarchyTreeNode* prevAddedAfter = HierarchyTreeController::Instance()->GetTree().GetNode(iter->second.addedAfter);
		QString prevName = iter->second.baseName;
		
		if (!currentNode || !prevParentNode)
		{
			continue;
		}
		
		currentNode->SetName(prevName);
		currentNode->SetParent(prevParentNode, prevAddedAfter);
	}
	
	HierarchyTreeController::Instance()->EmitHierarchyTreeUpdated();
}

void ChangeNodeHeirarchy::IncrementUnsavedChanges()
{
	HierarchyTreeNode* sourceNode = 0;
	HierarchyTreeNode* targetNode = HierarchyTreeController::Instance()->GetTree().GetNode(targetNodeID);

	switch (this->type)
	{
		case TYPE_PLATFORM:
			// The Platform nodes could only be moved within root node
			targetNode->IncrementUnsavedChanges();
			break;

		case TYPE_SCREEN:
		case TYPE_AGGREGATOR:
			// If Screen nodes are moved within one platform, it's enough to increment unsaved changes only in this platform...
			targetNode->IncrementUnsavedChanges();

			// ...Otherwise we need more complicated handling
			sourceNode = HierarchyTreeController::Instance()->GetTree().GetNode(items.front());
			if (sourceNode != targetNode)
			{
				sourceNode->IncrementUnsavedChanges();

				// Need to mark all screens which being moved to other platform.
				// Otherwise they won't be saved to files at the new location
				// Before mark, need to store current mark state for correct rollback.
				storedMarks.clear();
				for (HierarchyTreeNode::HIERARCHYTREENODESIDLIST::iterator it = items.begin(); it != items.end(); ++it)
				{
					HierarchyTreeNode* node = HierarchyTreeController::Instance()->GetTree().GetNode(*it);
					storedMarks[*it] = node->IsMarked();

					node->SetMarked(true);
				}
			}

			break;

		case TYPE_CONTROLS:
			targetNode->IncrementUnsavedChanges();

			sourceNode = HierarchyTreeController::Instance()->GetScreenNodeForNode(HierarchyTreeController::Instance()->GetTree().GetNode(items.front()));
			if (sourceNode != targetNode)
			{
				sourceNode->IncrementUnsavedChanges();
			}
			break;

		default:
			break;
	}
}

void ChangeNodeHeirarchy::DecrementUnsavedChanges()
{
	HierarchyTreeNode* sourceNode = 0;
	HierarchyTreeNode* targetNode = HierarchyTreeController::Instance()->GetTree().GetNode(targetNodeID);

	switch (this->type)
	{
		case TYPE_PLATFORM:
			targetNode->DecrementUnsavedChanges();
			break;

		case TYPE_SCREEN:
		case TYPE_AGGREGATOR:
			targetNode->DecrementUnsavedChanges();

			sourceNode = HierarchyTreeController::Instance()->GetTree().GetNode(items.front());
			if (sourceNode != targetNode)
			{
				sourceNode->DecrementUnsavedChanges();
			}

			// Restore mark state. See comments in IncrementUnsavedChanges()
			for (HierarchyTreeNode::HIERARCHYTREENODESIDLIST::iterator it = items.begin(); it != items.end(); ++it)
			{
				HierarchyTreeNode* node = HierarchyTreeController::Instance()->GetTree().GetNode(*it);

				if (storedMarks.find(*it) != storedMarks.end())
				{
					node->SetMarked(storedMarks[*it]);
				}
			}
			break;

		case TYPE_CONTROLS:
			targetNode->DecrementUnsavedChanges();

			sourceNode = HierarchyTreeController::Instance()->GetScreenNodeForNode(HierarchyTreeController::Instance()->GetTree().GetNode(items.front()));
			if (sourceNode != targetNode)
			{
				sourceNode->DecrementUnsavedChanges();
			}
			break;

		default:
			break;
	}
}
