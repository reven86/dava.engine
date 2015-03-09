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


#include "PasteCommand.h"
#include "HierarchyTreeAggregatorControlNode.h"
#include "CopyPasteHelper.h"

#define COPY_DELTA Vector2(5, 5)

PasteCommand::PasteCommand(HierarchyTreeNode* parentNode, CopyPasteController::CopyType copyType,
							const HierarchyTreeNode::HIERARCHYTREENODESLIST* items)
{
	this->parentNode = parentNode;
	this->copyType = copyType;
	this->items = items;
	this->newItems = NULL;
}

PasteCommand::~PasteCommand()
{
	// Nothing is to be cleaned up - Pasted Items are under control of Hierarchy Tree.
}

void PasteCommand::Execute()
{
	HierarchyTreeController::Instance()->ResetSelectedControl();
	
	int count = 0;
	
	if (this->newItems)
	{
		// We are performing Rollback after Execute.
		ReturnPastedControlsToScene();
		return;
	}

	// This is the first Execute - remember the items to be pasted.
	this->newItems = new HierarchyTreeNode::HIERARCHYTREENODESLIST();

	switch (copyType) {
		case CopyPasteController::CopyTypeControl:
		{
			if (parentNode)
			{
                count += PasteControls(newItems, parentNode);
				for (HierarchyTreeNode::HIERARCHYTREENODESLIST::iterator iter = newItems->begin();
					 iter != newItems->end();
					 ++iter)
				{
					HierarchyTreeNode* node = (*iter);
					HierarchyTreeControlNode* controlNode = dynamic_cast<HierarchyTreeControlNode* >(node);
					if (!controlNode)
						continue;
					
					parentNode->AddTreeNode(controlNode);
                    //Check type of parent control
                    //For screen control there is another sequence
                    HierarchyTreeScreenNode* parentScreen = dynamic_cast<HierarchyTreeScreenNode*>(parentNode);
                    HierarchyTreeControlNode* parentControl = dynamic_cast<HierarchyTreeControlNode *>(parentNode);
                    if (parentScreen)
                    {
                        parentScreen->GetScreen()->AddControl(controlNode->GetUIObject());
                    }
                    else if (parentControl)
                    {
                        parentControl->GetUIObject()->AddControl(controlNode->GetUIObject()); 
                    }
					
					HierarchyTreeController::Instance()->ChangeItemSelection(controlNode);
				}
			}
		}break;
		case CopyPasteController::CopyTypeScreen:
		case CopyPasteController::CopyTypeAggregator:
		{
			HierarchyTreePlatformNode* parentPlatform = dynamic_cast<HierarchyTreePlatformNode*>(parentNode);
			if (parentPlatform)
			{
				count += PasteScreens(newItems, parentPlatform);
				for (HierarchyTreeNode::HIERARCHYTREENODESLIST::iterator iter = newItems->begin();
					 iter != newItems->end();
					 ++iter)
				{
					HierarchyTreeNode* node = (*iter);
					HierarchyTreeScreenNode* screenNode = dynamic_cast<HierarchyTreeScreenNode* >(node);
					if (!screenNode)
						continue;
					
					parentPlatform->AddTreeNode(screenNode);
				}
			}
		}break;
		case CopyPasteController::CopyTypePlatform:
		{
			HierarchyTreeRootNode* parentRoot = dynamic_cast<HierarchyTreeRootNode*>(parentNode);
			if (parentRoot)
			{
				count += PastePlatforms(newItems, parentRoot);
				for (HierarchyTreeNode::HIERARCHYTREENODESLIST::iterator iter = newItems->begin();
					 iter != newItems->end();
					 ++iter)
				{
					HierarchyTreeNode* node = (*iter);
					HierarchyTreePlatformNode* platformNode = dynamic_cast<HierarchyTreePlatformNode* >(node);
					if (!platformNode)
						continue;
					
					parentRoot->AddTreeNode(platformNode);
				}
			}
		}break;
			
		default:
			break;
	}
	
	HierarchyTreeController::Instance()->EmitHierarchyTreeUpdated();
}

void PasteCommand::Rollback()
{
	if (!newItems)
	{
		return;
	}
	
	// Cleanup the new items, if any.
	for (HierarchyTreeNode::HIERARCHYTREENODESLIST::iterator iter = this->newItems->begin();
		 iter != this->newItems->end(); ++iter)
	{
		// Remove the controls added and appropriate nodes. Don't delete them from
		// memory - we might return them in Rollback.
		HierarchyTreeNode* curNode = (*iter);
		curNode->PrepareRemoveFromSceneInformation();
		HierarchyTreeController::Instance()->DeleteNode(curNode->GetId(), false, true, true);
	}

	HierarchyTreeController::Instance()->EmitHierarchyTreeUpdated();
}

int PasteCommand::PasteControls(HierarchyTreeNode::HIERARCHYTREENODESLIST* newControls, HierarchyTreeNode *parent)
{
	int count = 0;
	for (HierarchyTreeNode::HIERARCHYTREENODESCONSTITER iter = items->begin();
		 iter != items->end();
		 ++iter)
	{
		HierarchyTreeNode *node = (*iter);
		HierarchyTreeControlNode* control = dynamic_cast<HierarchyTreeControlNode*>(node);
		if (!control)
			continue;
			
		//  We should change control name and apply copy delta only if new parent already has children with such name
		bool bUpdateNameAndShiftPosition = IsParentContainsCopyItemName(parent, control);

        CopyPasteHelper::UpdateAggregators(control, parent);
  		
		HierarchyTreeControlNode* copy = control->CreateControlCopy(parent);
			
		UpdateControlName(parent, copy, bUpdateNameAndShiftPosition);

		UIControl* clone = copy->GetUIObject();
		if (clone && bUpdateNameAndShiftPosition)
		{
			HierarchyTreeNode::AlignData alignData = HierarchyTreeNode::SaveAlignData(clone);
			clone->SetPosition(clone->GetPosition() + COPY_DELTA);
			HierarchyTreeNode::RestoreAlignData(clone, alignData);
		}

		count++;
		newControls->push_back(copy);
	}
	
	return count;
}

bool PasteCommand::IsParentContainsCopyItemName(HierarchyTreeNode* parentNode, HierarchyTreeNode* copyNode)
{
	const QString copyName = copyNode->GetName();
	
	for (HierarchyTreeNode::HIERARCHYTREENODESCONSTITER iter = parentNode->GetChildNodes().begin();
			 iter != parentNode->GetChildNodes().end();
			 ++iter)
	{
		HierarchyTreeControlNode* control = dynamic_cast<HierarchyTreeControlNode*>((*iter));
		if (!control)
			continue;
				
		if (control->GetName().compare(copyName) == 0)
		{
			return true;
		}
	}
	
	return false;
}

void PasteCommand::UpdateControlName(const HierarchyTreeNode* parent, HierarchyTreeNode* node, bool needCreateNewName) const
{
	QString name = node->GetName();
	if (needCreateNewName)
	{
		name = CopyPasteHelper::FormatCopyName(node->GetName(), parent);
	}
	node->SetName(name);
    
    // Also need to update the name of the UIControl - it is not copied.
    HierarchyTreeControlNode* controlNode = dynamic_cast<HierarchyTreeControlNode*>(node);
    if (controlNode && controlNode->GetUIObject())
    {
        controlNode->GetUIObject()->SetName(node->GetName().toStdString());
    }

	const HierarchyTreeNode::HIERARCHYTREENODESLIST &child = node->GetChildNodes();
	for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter = child.begin();
		 iter != child.end();
		 ++iter)
	{
		HierarchyTreeNode* child = (*iter);
		UpdateControlName(parent, child, false);
	}
}

int PasteCommand::PasteScreens(HierarchyTreeNode::HIERARCHYTREENODESLIST* newScreens, HierarchyTreePlatformNode* parent)
{
	int count = 0;
	for (HierarchyTreeNode::HIERARCHYTREENODESCONSTITER iter = items->begin();
		 iter != items->end();
		 ++iter)
	{
		HierarchyTreeNode *node = (*iter);
		HierarchyTreeScreenNode* screen = dynamic_cast<HierarchyTreeScreenNode*>(node);
		HierarchyTreeAggregatorNode* aggregator = dynamic_cast<HierarchyTreeAggregatorNode*>(node);

		if (!screen && !aggregator)
			continue;
        
        CopyPasteHelper::UpdateAggregators(screen, parent);

		HierarchyTreeNode* copy;
		if (aggregator)
		{
			copy = new HierarchyTreeAggregatorNode(parent, aggregator, false);
		}
		else
		{
			copy = new HierarchyTreeScreenNode(parent, screen, false);
		}
		copy->SetMarked(true);
        UpdateControlName(parent, copy, parent->IsAggregatorOrScreenNamePresent(copy->GetName()));
		//copy->SetName(FormatCopyName(screen->GetName(), parent));
		
		count++;
		newScreens->push_back(copy);
	}
	
	return count;
}

int PasteCommand::PastePlatforms(HierarchyTreeNode::HIERARCHYTREENODESLIST* newScreens, HierarchyTreeRootNode* parent)
{
	int count = 0;
	for (HierarchyTreeNode::HIERARCHYTREENODESCONSTITER iter = items->begin();
		 iter != items->end();
		 ++iter)
	{
		HierarchyTreeNode *node = (*iter);
		HierarchyTreePlatformNode* platform = dynamic_cast<HierarchyTreePlatformNode*>(node);
		if (!platform)
			continue;
		
		HierarchyTreePlatformNode* copy = new HierarchyTreePlatformNode(parent, platform);

		copy->SetMarked(true);
		copy->SetChildrenMarked(true);

        UpdateControlName(parent, copy, true);
		//copy->SetName(FormatCopyName(platform->GetName(), parent));
		
		count++;
		newScreens->push_back(copy);
	}
	
	return count;
}

void PasteCommand::ReturnPastedControlsToScene()
{
	if (!newItems)
	{
		return;
	}

	for (HierarchyTreeNode::HIERARCHYTREENODESLIST::iterator iter = this->newItems->begin();
		 iter != this->newItems->end(); ++iter)
	{
		// Return the removed node back to scene.
		HierarchyTreeNode* curNode = (*iter);
		HierarchyTreeController::Instance()->ReturnNodeToScene(curNode);
	}

	HierarchyTreeController::Instance()->EmitHierarchyTreeUpdated();
}

void PasteCommand::IncrementUnsavedChanges()
{
	switch (this->copyType)
	{
		case CopyPasteController::CopyTypeControl:
			BaseCommand::IncrementUnsavedChanges();
			break;

		case CopyPasteController::CopyTypeScreen:
		case CopyPasteController::CopyTypeAggregator:
		case CopyPasteController::CopyTypePlatform:
			parentNode->IncrementUnsavedChanges();
			break;

		default:
			break;
	}
}

void PasteCommand::DecrementUnsavedChanges()
{
	switch (this->copyType)
	{
		case CopyPasteController::CopyTypeControl:
			BaseCommand::DecrementUnsavedChanges();
			break;

		case CopyPasteController::CopyTypeScreen:
		case CopyPasteController::CopyTypeAggregator:
		case CopyPasteController::CopyTypePlatform:
			parentNode->DecrementUnsavedChanges();
			break;

		default:
			break;
	}
}